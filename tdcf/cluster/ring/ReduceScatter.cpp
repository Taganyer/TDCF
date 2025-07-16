//
// Created by taganyer on 25-7-9.
//

#include <tdcf/base/Errors.hpp>
#include <tdcf/base/types/Ring.hpp>
#include <tdcf/cluster/ring/RingCluster.hpp>

using namespace tdcf;

using namespace tdcf::ring;

RingCluster::ReduceScatter::ReduceScatter(ProgressType type, uint32_t version, ProcessingRulesPtr rp) :
    EventProgress(OperationType::ReduceScatter, type, version, std::move(rp)) {}

StatusFlag RingCluster::ReduceScatter::create(ProcessingRulesPtr rp, Handle& handle) {
    uint32_t version = handle.create_progress_version();
    auto iter = handle.create_progress(
        std::make_unique<ReduceScatter>(ProgressType::Root, version, std::move(rp)));

    auto& [send, receive, cluster_size] = handle.cluster_data<RingClusterData>();
    auto& self = static_cast<ReduceScatter&>(*iter->second);
    self._self = iter;
    self.serial = cluster_size;

    MetaData meta = self.create_meta();
    meta.stage = C_ReduceScatter::acquire_data;
    handle.acquire_data(iter, meta, self.rule);

    meta.stage = C_ReduceScatter::send_rule;
    StatusFlag flag = handle.send_progress_message(version, send, meta, self.rule);
    TDCF_CHECK_SUCCESS(flag)

    return StatusFlag::Success;
}

StatusFlag RingCluster::ReduceScatter::handle_event(const MetaData& meta,
                                                    Variant& data, Handle& handle) {
    assert(meta.operation_type == OperationType::ReduceScatter);
    if (meta.stage == C_ReduceScatter::acquire_data) {
        return acquire_data(std::get<DataSet>(data), handle);
    }
    if (meta.stage == C_ReduceScatter::get_data) {
        return scatter_data(std::get<DataPtr>(data), meta.rest_data, handle);
    }
    if (meta.stage == C_ReduceScatter::scatter_data) {
        return send_data(std::get<DataSet>(data), handle);
    }
    if (meta.stage == C_ReduceScatter::finish_ack) {
        rule->finish_callback();
        return StatusFlag::EventEnd;
    }
    TDCF_RAISE_ERROR(meta.stage error type)
}

StatusFlag RingCluster::ReduceScatter::acquire_data(DataSet& dataset, Handle& handle) const {
    auto& [send, receive, cluster_size] = handle.cluster_data<RingClusterData>();
    MetaData meta = create_meta();
    meta.stage = C_ReduceScatter::send_data1;
    meta.rest_data = dataset.size();

    for (auto& data : dataset) {
        --meta.rest_data;
        StatusFlag flag = handle.send_progress_message(version, send, meta, data);
        TDCF_CHECK_SUCCESS(flag)
    }

    return StatusFlag::Success;
}

StatusFlag RingCluster::ReduceScatter::scatter_data(DataPtr& data,
                                                    uint32_t rest_size, Handle& handle) {
    _set.emplace_back(std::move(data));
    if (rest_size == 0) {
        auto& [send, receive, cluster_size] = handle.cluster_data<RingClusterData>();
        MetaData meta = create_meta();
        meta.stage = C_ReduceScatter::scatter_data;
        handle.scatter_data(_self, meta, rule, cluster_size + 1, std::move(_set));
    }
    return StatusFlag::Success;
}

StatusFlag RingCluster::ReduceScatter::send_data(DataSet& set, Handle& handle) const {
    auto& [send, receive, cluster_size] = handle.cluster_data<RingClusterData>();
    uint32_t patch = set.size() / (cluster_size + 1);
    TDCF_CHECK_EXPR(patch > 0 && set.size() % (cluster_size + 1) == 0);

    MetaData meta = create_meta();
    meta.stage = C_ReduceScatter::send_data2;
    meta.rest_data = patch;

    for (uint32_t i = 0; i < patch; ++i) {
        handle.store_data(rule, set[i]);
        --meta.rest_data;
        for (uint32_t j = i + patch; j < set.size(); j += patch) {
            StatusFlag flag = handle.send_progress_message(version, send, meta, std::move(set[j]));
            TDCF_CHECK_SUCCESS(flag)
        }
    }

    meta.stage = C_ReduceScatter::finish;
    StatusFlag flag = handle.send_progress_message(version, send, meta, nullptr);
    TDCF_CHECK_SUCCESS(flag)
    return StatusFlag::Success;
}

RingCluster::ReduceScatterAgent::ReduceScatterAgent(uint32_t version, ProcessingRulesPtr rp, ProgressEventsMI iter) :
    ReduceScatter(ProgressType::NodeRoot, version, std::move(rp)), _other(iter) {}

StatusFlag RingCluster::ReduceScatterAgent::create(ProcessingRulesPtr rp, ProgressEventsMI other,
                                                   Handle& handle, EventProgressAgent **agent_ptr) {
    uint32_t version = handle.create_progress_version();
    auto iter = handle.create_progress(
        std::make_unique<ReduceScatterAgent>(version, std::move(rp), other));

    auto& [send, receive, cluster_size] = handle.cluster_data<RingClusterData>();
    auto& self = static_cast<ReduceScatterAgent&>(*iter->second);
    *agent_ptr = &self;
    self._self = iter;
    self.serial = cluster_size;

    MetaData meta = self.create_meta();
    meta.stage = A_ReduceScatter::acquire_data;
    handle.acquire_data(iter, meta, self.rule);

    meta.stage = A_ReduceScatter::send_rule;
    StatusFlag flag = handle.send_progress_message(version, send, meta, self.rule);
    TDCF_CHECK_SUCCESS(flag)

    return StatusFlag::Success;
}

StatusFlag RingCluster::ReduceScatterAgent::handle_event(const MetaData& meta,
                                                         Variant& data, Handle& handle) {
    assert(meta.operation_type == OperationType::ReduceScatter);
    if (meta.stage == A_ReduceScatter::acquire_data) {
        return acquire_data(std::get<DataSet>(data), handle);
    }
    if (meta.stage == A_ReduceScatter::get_data) {
        return agent_get_data(std::get<DataPtr>(data), meta.rest_data, handle);
    }
    if (meta.stage == Public_ReduceScatter::agent_receive) {
        auto& set = std::get<DataSet>(data);
        uint32_t rest_size = set.size();
        for (auto& d : set) {
            --rest_size;
            scatter_data(d, rest_size, handle);
        }
        return StatusFlag::Success;
    }
    if (meta.stage == C_ReduceScatter::scatter_data) {
        return send_data(std::get<DataSet>(data), handle);
    }
    if (meta.stage == C_ReduceScatter::finish_ack) {
        return close(handle);
    }
    TDCF_RAISE_ERROR(meta.stage error type)
}

StatusFlag RingCluster::ReduceScatterAgent::proxy_event(const MetaData& meta,
                                                        Variant& data, Handle& handle) {
    return handle_event(meta, data, handle);
}

StatusFlag RingCluster::ReduceScatterAgent::agent_get_data(DataPtr& data,
                                                           uint32_t rest_size, Handle& handle) {
    _set.emplace_back(std::move(data));
    if (rest_size == 0) {
        auto& [send, receive, cluster_size] = handle.cluster_data<RingClusterData>();
        MetaData meta = create_meta();
        meta.stage = Public_ReduceScatter::agent_send;
        handle.create_processor_event(_other, meta, std::move(_set));
    }
    return StatusFlag::Success;
}

StatusFlag RingCluster::ReduceScatterAgent::close(Handle& handle) const {
    MetaData meta = create_meta();
    meta.stage = Public_ReduceScatter::agent_finish;
    handle.create_processor_event(_other, meta, SerializablePtr());
    return StatusFlag::EventEnd;
}
