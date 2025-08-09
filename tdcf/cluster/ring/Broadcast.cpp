//
// Created by taganyer on 25-7-8.
//

#include <tdcf/base/Errors.hpp>
#include <tdcf/base/types/Ring.hpp>
#include <tdcf/cluster/ring/RingCluster.hpp>

using namespace tdcf;

using namespace tdcf::ring;

RingCluster::Broadcast::Broadcast(ProgressType type, uint32_t version, ProcessingRulesPtr rp) :
    EventProgress(OperationType::Broadcast, type, version, std::move(rp)) {}

StatusFlag RingCluster::Broadcast::create(ProcessingRulesPtr rp, Handle& handle) {
    uint32_t version = handle.create_progress_version();
    auto iter = handle.create_progress(
        std::make_unique<Broadcast>(ProgressType::Root, version, std::move(rp)));

    auto& [send, receive, cluster_size] = handle.cluster_data<RingClusterData>();
    auto& self = static_cast<Broadcast&>(*iter->second);
    self.serial = cluster_size;

    MetaData meta = self.create_meta();
    meta.stage = C_Broadcast::acquire_data;
    handle.acquire_data(iter, meta, self.rule);

    meta.stage = C_Broadcast::send_rule;
    StatusFlag flag = handle.send_progress_message(version, send, meta, self.rule);
    TDCF_CHECK_SUCCESS(flag)

    return StatusFlag::Success;
}

StatusFlag RingCluster::Broadcast::handle_event(const MetaData& meta,
                                                Variant& data, Handle& handle) {
    assert(meta.operation_type == OperationType::Broadcast);
    if (meta.stage == C_Broadcast::acquire_data) {
        return send_data(std::get<DataSet>(data), handle);
    }
    if (meta.stage == C_Broadcast::finish_ack) {
        assert(meta.serial == 0);
        return StatusFlag::EventEnd;
    }
    TDCF_RAISE_ERROR(meta.stage error type)
}

StatusFlag RingCluster::Broadcast::send_data(DataSet& dataset, Handle& handle) const {
    auto& send = handle.cluster_data<RingClusterData>().send;
    MetaData meta = create_meta();
    meta.stage = C_Broadcast::send_data;
    meta.rest_data = dataset.size();

    for (auto& data : dataset) {
        --meta.rest_data;
        StatusFlag flag = handle.send_progress_message(version, send, meta, data);
        TDCF_CHECK_SUCCESS(flag)
    }

    meta.stage = C_Broadcast::finish_ack;
    StatusFlag flag = handle.send_progress_message(version, send, meta, nullptr);
    TDCF_CHECK_SUCCESS(flag)
    return StatusFlag::Success;
}

RingCluster::BroadcastAgent::BroadcastAgent(uint32_t version, ProcessingRulesPtr rp,
                                            ProgressEventsMI iter) :
    Broadcast(ProgressType::NodeRoot, version, std::move(rp)), _other(iter) {}

StatusFlag RingCluster::BroadcastAgent::create(ProcessingRulesPtr rp, ProgressEventsMI other,
                                               Handle& handle, EventProgressAgent **agent_ptr) {
    uint32_t version = handle.create_progress_version();
    auto iter = handle.create_progress(
        std::make_unique<BroadcastAgent>(version, std::move(rp), other));

    auto& [send, receive, cluster_size] = handle.cluster_data<RingClusterData>();
    auto& self = static_cast<BroadcastAgent&>(*iter->second);
    self.serial = cluster_size;
    *agent_ptr = &self;

    MetaData meta = self.create_meta();
    meta.stage = A_Broadcast::send_rule;
    StatusFlag flag = handle.send_progress_message(version, send, meta, self.rule);
    TDCF_CHECK_SUCCESS(flag)

    return StatusFlag::Success;
}

StatusFlag RingCluster::BroadcastAgent::handle_event(const MetaData& meta,
                                                     Variant& data, Handle& handle) {
    assert(meta.operation_type == OperationType::Broadcast);
    if (meta.stage == Public_Broadcast::agent_receive) {
        return send_data(std::get<DataSet>(data), handle);
    }
    if (meta.stage == A_Broadcast::finish_ack) {
        assert(meta.serial == 0);
        return close(handle);
    }
    TDCF_RAISE_ERROR(meta.stage error type)
}

StatusFlag RingCluster::BroadcastAgent::proxy_event(const MetaData& meta,
                                                    Variant& data, Handle& handle) {
    return handle_event(meta, data, handle);
}

StatusFlag RingCluster::BroadcastAgent::close(Handle& handle) const {
    MetaData meta = create_meta();
    meta.stage = Public_Broadcast::agent_finish;
    handle.create_processor_event(_other, meta, nullptr);
    return StatusFlag::EventEnd;
}
