//
// Created by taganyer on 25-7-8.
//

#include <tdcf/base/Errors.hpp>
#include <tdcf/base/types/Ring.hpp>
#include <tdcf/cluster/ring/RingCluster.hpp>

using namespace tdcf;

using namespace tdcf::ring;

RingCluster::Reduce::Reduce(ProgressType type, uint32_t version, ProcessingRulesPtr rp) :
    EventProgress(OperationType::Reduce, type, version, std::move(rp)) {}

StatusFlag RingCluster::Reduce::create(ProcessingRulesPtr rp, Handle& handle) {
    uint32_t version = handle.create_progress_version();
    auto iter = handle.create_progress(
        std::make_unique<Reduce>(ProgressType::Root, version, std::move(rp)));

    auto& [send, receive, cluster_size] = handle.cluster_data<RingClusterData>();

    auto& self = static_cast<Reduce&>(*iter->second);

    MetaData meta = self.create_meta();
    meta.stage = C_Reduce::acquire_data;
    handle.acquire_data(iter, meta, self.rule);

    meta.stage = C_Reduce::send_rule;
    StatusFlag flag = handle.send_progress_message(version, send, meta, self.rule);
    TDCF_CHECK_SUCCESS(flag)

    return StatusFlag::Success;
}

StatusFlag RingCluster::Reduce::handle_event(const MetaData& meta,
                                             Variant& data, Handle& handle) {
    assert(meta.operation_type == OperationType::Reduce);
    if (meta.stage == C_Reduce::acquire_data) {
        return acquire_data(std::get<DataPtr>(data), handle);
    }
    if (meta.stage == C_Reduce::get_data) {
        handle.store_data(rule, std::get<DataPtr>(data));
        rule->finish_callback();
        return StatusFlag::EventEnd;
    }
    TDCF_RAISE_ERROR(meta.stage error type)
}

StatusFlag RingCluster::Reduce::acquire_data(DataPtr& data, Handle& handle) const {
    auto& [send, receive, cluster_size] = handle.cluster_data<RingClusterData>();
    MetaData meta = create_meta();
    meta.stage = C_Reduce::send_data;
    StatusFlag flag = handle.send_progress_message(version, send, meta, data);
    TDCF_CHECK_SUCCESS(flag)
    return StatusFlag::Success;
}

RingCluster::ReduceAgent::ReduceAgent(uint32_t version, ProcessingRulesPtr rp, ProgressEventsMI iter) :
    Reduce(ProgressType::NodeRoot, version, std::move(rp)), _other(iter) {}

StatusFlag RingCluster::ReduceAgent::create(ProcessingRulesPtr rp, ProgressEventsMI other,
                                            Handle& handle, EventProgressAgent **agent_ptr) {

    uint32_t version = handle.create_progress_version();
    auto iter = handle.create_progress(
        std::make_unique<ReduceAgent>(version, std::move(rp), other));

    auto& [send, receive, cluster_size] = handle.cluster_data<RingClusterData>();

    auto& self = static_cast<ReduceAgent&>(*iter->second);
    *agent_ptr = &self;

    MetaData meta = self.create_meta();
    meta.stage = A_Reduce::acquire_data;
    handle.acquire_data(iter, meta, self.rule);

    meta.stage = A_Reduce::send_rule;
    StatusFlag flag = handle.send_progress_message(version, send, meta, self.rule);
    TDCF_CHECK_SUCCESS(flag)

    return StatusFlag::Success;
}

StatusFlag RingCluster::ReduceAgent::handle_event(const MetaData& meta,
                                                  Variant& data, Handle& handle) {
    assert(meta.operation_type == OperationType::Reduce);
    if (meta.stage == A_Reduce::acquire_data) {
        return acquire_data(std::get<DataPtr>(data), handle);
    }
    if (meta.stage == A_Reduce::get_data) {
        return close(std::get<DataPtr>(data), handle);
    }
    TDCF_RAISE_ERROR(meta.stage error type)
}

StatusFlag RingCluster::ReduceAgent::proxy_event(const MetaData& meta,
                                                 Variant& data, Handle& handle) {
    return handle_event(meta, data, handle);
}

StatusFlag RingCluster::ReduceAgent::close(DataPtr& data, Handle& handle) const {
    MetaData meta = create_meta();
    meta.stage = Public_Reduce::agent_send;
    handle.create_processor_event(_other, meta, std::static_pointer_cast<Serializable>(data));
    return StatusFlag::EventEnd;
}
