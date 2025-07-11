//
// Created by taganyer on 25-7-8.
//

#include <tdcf/base/Errors.hpp>
#include <tdcf/base/types/Ring.hpp>
#include <tdcf/cluster/ring/RingCluster.hpp>

using namespace tdcf;

using namespace tdcf::ring;

RingCluster::Scatter::Scatter(ProgressType type, uint32_t version, ProcessingRulesPtr rp) :
    EventProgress(OperationType::Scatter, type, version, std::move(rp)) {}

StatusFlag RingCluster::Scatter::create(ProcessingRulesPtr rp, Handle& handle) {
    uint32_t version = handle.create_progress_version();
    auto iter = handle.create_progress(
        std::make_unique<Scatter>(ProgressType::Root, version, std::move(rp)));

    auto& [send, receive, cluster_size] = handle.cluster_data<RingClusterData>();
    auto& self = static_cast<Scatter&>(*iter->second);
    self.serial = cluster_size;
    self._self = iter;

    MetaData meta = self.create_meta();
    meta.stage = C_Scatter::acquire_data;
    handle.acquire_data(iter, meta, self.rule);

    meta.stage = C_Scatter::send_rule;
    StatusFlag flag = handle.send_progress_message(version, send, meta, self.rule);
    TDCF_CHECK_SUCCESS(flag)

    return StatusFlag::Success;
}

StatusFlag RingCluster::Scatter::handle_event(const MetaData& meta, Variant& data, Handle& handle) {
    assert(meta.operation_type == OperationType::Scatter);
    if (meta.stage == C_Scatter::acquire_data) {
        return scatter_data(std::get<DataSet>(data), handle);
    }
    if (meta.stage == C_Scatter::scatter_data) {
        return send_data(std::get<DataSet>(data), handle);
    }
    if (meta.stage == C_Scatter::finish_ack) {
        rule->finish_callback();
        return StatusFlag::EventEnd;
    }
    TDCF_RAISE_ERROR(meta.stage error type)
}

StatusFlag RingCluster::Scatter::scatter_data(DataSet& dataset, Handle& handle) const {
    auto& [send, receive, cluster_size] = handle.cluster_data<RingClusterData>();
    MetaData meta = create_meta();
    meta.stage = C_Scatter::scatter_data;
    handle.scatter_data(_self, meta, rule, cluster_size + 1, dataset);
    return StatusFlag::Success;
}

StatusFlag RingCluster::Scatter::send_data(DataSet& set, Handle& handle) const {
    auto& [send, receive, cluster_size] = handle.cluster_data<RingClusterData>();
    uint32_t patch = set.size() / (cluster_size + 1);
    TDCF_CHECK_EXPR(patch > 0 && set.size() % (cluster_size + 1) == 0);

    MetaData meta = create_meta();
    meta.stage = C_Scatter::send_data;
    meta.rest_data = patch;

    for (uint32_t i = 0; i < patch; ++i) {
        handle.store_data(rule, set[i]);
        --meta.rest_data;
        for (uint32_t j = i + patch; j < set.size(); j += patch) {
            StatusFlag flag = handle.send_progress_message(version, send, meta, std::move(set[j]));
            TDCF_CHECK_SUCCESS(flag)
        }
    }

    meta.stage = C_Scatter::finish;
    handle.send_progress_message(version, send, meta, nullptr);

    return StatusFlag::Success;
}


RingCluster::ScatterAgent::ScatterAgent(uint32_t version, ProcessingRulesPtr rp, ProgressEventsMI iter) :
    Scatter(ProgressType::NodeRoot, version, std::move(rp)), _other(iter) {}

StatusFlag RingCluster::ScatterAgent::create(ProcessingRulesPtr rp, ProgressEventsMI other,
                                             Handle& handle, EventProgressAgent **agent_ptr) {
    uint32_t version = handle.create_progress_version();
    auto iter = handle.create_progress(
        std::make_unique<ScatterAgent>(version, std::move(rp), other));

    auto& [send, receive, cluster_size] = handle.cluster_data<RingClusterData>();
    auto& self = static_cast<ScatterAgent&>(*iter->second);
    *agent_ptr = &self;
    self.serial = cluster_size;
    self._self = iter;

    MetaData meta = self.create_meta();
    meta.stage = A_Scatter::send_rule;
    StatusFlag flag = handle.send_progress_message(version, send, meta, self.rule);
    TDCF_CHECK_SUCCESS(flag)

    return StatusFlag::Success;
}

StatusFlag RingCluster::ScatterAgent::handle_event(const MetaData& meta,
                                                   Variant& data, Handle& handle) {
    assert(meta.operation_type == OperationType::Scatter);
    if (meta.stage == Public_Scatter::agent_receive) {
        return scatter_data(std::get<DataSet>(data), handle);
    }
    if (meta.stage == A_Scatter::scatter_data) {
        return send_data(std::get<DataSet>(data), handle);
    }
    if (meta.stage == A_Scatter::finish_ack) {
        return close(handle);
    }
    TDCF_RAISE_ERROR(meta.stage error type)
}

StatusFlag RingCluster::ScatterAgent::proxy_event(const MetaData& meta,
                                                  Variant& data, Handle& handle) {
    return handle_event(meta, data, handle);
}

StatusFlag RingCluster::ScatterAgent::close(Handle& handle) const {
    MetaData meta = create_meta();
    meta.stage = Public_Scatter::agent_finish;
    handle.create_processor_event(_other, meta, nullptr);
    return StatusFlag::EventEnd;
}
