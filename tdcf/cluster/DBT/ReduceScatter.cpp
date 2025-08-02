//
// Created by taganyer on 25-7-31.
//

#include <tdcf/base/Errors.hpp>
#include <tdcf/base/types/DBT.hpp>
#include <tdcf/cluster/DBT/DBTCluster.hpp>

using namespace tdcf;

using namespace tdcf::dbt;

DBTCluster::ReduceScatter::ReduceScatter(ProgressType type, uint32_t version, ProcessingRulesPtr rp) :
    EventProgress(OperationType::ReduceScatter, type, version, std::move(rp)) {}

StatusFlag DBTCluster::ReduceScatter::create(ProcessingRulesPtr rp, Handle& handle) {
    uint32_t version = handle.create_progress_version();
    auto iter = handle.create_progress(
        std::make_unique<ReduceScatter>(ProgressType::Root, version, std::move(rp)));

    auto& [t1, t2, size] = handle.cluster_data<DBTClusterData>();

    auto& self = static_cast<ReduceScatter&>(*iter->second);
    self._self = iter;

    MetaData meta = self.create_meta();
    meta.stage = N_ReduceScatter::self_acquire_data;
    handle.acquire_data(iter, meta, self.rule);

    meta.stage = N_ReduceScatter::send_rule;
    StatusFlag flag = handle.send_progress_message(version, t1, meta, self.rule);
    TDCF_CHECK_SUCCESS(flag)
    flag = handle.send_progress_message(version, t2, meta, self.rule);
    TDCF_CHECK_SUCCESS(flag)

    return StatusFlag::Success;
}

StatusFlag DBTCluster::ReduceScatter::handle_event(const MetaData& meta,
                                                   Variant& data, Handle& handle) {
    assert(meta.operation_type == OperationType::ReduceScatter);
    if (meta.stage == C_ReduceScatter::self_acquire_data) {
        auto& set = std::get<DataSet>(data);
        uint32_t rest_size = set.size();
        for (auto& d : set) {
            --rest_size;
            acquire_data(d, rest_size, handle);
        }
        return StatusFlag::Success;
    }
    if (meta.stage == C_ReduceScatter::acquire_data) {
        return acquire_data(std::get<DataPtr>(data), meta.rest_data, handle);
    }
    if (meta.stage == C_ReduceScatter::reduce_data) {
        return scatter_data(std::get<DataSet>(data), handle);
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

StatusFlag DBTCluster::ReduceScatter::acquire_data(DataPtr& data,
                                                   uint32_t rest_size, Handle& handle) {
    if (data->derived_type() != 0) _set.emplace_back(std::move(data));
    if (rest_size == 0 && ++_received == 3) {
        MetaData meta = create_meta();
        meta.stage = C_ReduceScatter::reduce_data;
        handle.reduce_data(_self, meta, rule, std::move(_set));
    }
    return StatusFlag::Success;
}

StatusFlag DBTCluster::ReduceScatter::scatter_data(DataSet& dataset, Handle& handle) const {
    auto& [t1, t2, size] = handle.cluster_data<DBTClusterData>();
    MetaData meta = create_meta();
    meta.stage = C_ReduceScatter::scatter_data;
    handle.scatter_data(_self, meta, rule, size + 1, std::move(dataset));
    return StatusFlag::Success;
}

StatusFlag DBTCluster::ReduceScatter::send_data(DataSet& set, Handle& handle) const {
    auto& [t1, t2, size] = handle.cluster_data<DBTClusterData>();
    uint32_t patch = set.size() / (size + 1);
    TDCF_CHECK_EXPR(patch > 0 && set.size() % (size + 1) == 0);

    if (patch == 1) {
        DataSet tmp;
        tmp.reserve(set.size() * 2);
        for (auto& data : set) {
            tmp.emplace_back(std::move(data));
            tmp.emplace_back(DataPtr());
        }
        set = std::move(tmp);
        ++patch;
    }

    MetaData meta = create_meta();
    meta.stage = C_ReduceScatter::send_data;

    uint32_t t1_rest_data = (patch + 1) / 2, t2_rest_data = patch / 2;
    for (uint32_t i = 0; i < patch; ++i) {
        if (set[i]->derived_type() != 0) {
            handle.store_data(rule, set[i]);
        }
        if (i & 1) meta.rest_data = --t2_rest_data;
        else meta.rest_data = --t1_rest_data;
        for (uint32_t j = i + patch; j < set.size(); j += patch) {
            meta.serial = j / patch - 1;
            if (i & 1) {
                meta.data1[0] = 0;
                StatusFlag flag = handle.send_progress_message(version, t2, meta, set[j]);
                TDCF_CHECK_SUCCESS(flag)
            } else {
                meta.data1[0] = 1;
                StatusFlag flag = handle.send_progress_message(version, t1, meta, set[j]);
                TDCF_CHECK_SUCCESS(flag)
            }
        }
    }

    return StatusFlag::Success;
}

DBTCluster::ReduceScatterAgent::ReduceScatterAgent(uint32_t version, ProcessingRulesPtr rp, ProgressEventsMI iter) :
    ReduceScatter(ProgressType::NodeRoot, version, std::move(rp)), _other(iter) {}

StatusFlag DBTCluster::ReduceScatterAgent::create(ProcessingRulesPtr rp, ProgressEventsMI other,
                                                  Handle& handle, EventProgressAgent **agent_ptr) {
    uint32_t version = handle.create_progress_version();
    auto iter = handle.create_progress(
        std::make_unique<ReduceScatterAgent>(version, std::move(rp), other));

    auto& [t1, t2, size] = handle.cluster_data<DBTClusterData>();

    auto& self = static_cast<ReduceScatterAgent&>(*iter->second);
    *agent_ptr = &self;
    self._self = iter;

    MetaData meta = self.create_meta();
    meta.stage = A_ReduceScatter::self_acquire_data;
    handle.acquire_data(iter, meta, self.rule);

    meta.stage = A_ReduceScatter::send_rule;
    StatusFlag flag = handle.send_progress_message(version, t1, meta, self.rule);
    TDCF_CHECK_SUCCESS(flag)
    flag = handle.send_progress_message(version, t2, meta, self.rule);
    TDCF_CHECK_SUCCESS(flag)

    return StatusFlag::Success;
}

StatusFlag DBTCluster::ReduceScatterAgent::handle_event(const MetaData& meta,
                                                        Variant& data, Handle& handle) {
    assert(meta.operation_type == OperationType::ReduceScatter);
    if (meta.stage == A_ReduceScatter::self_acquire_data) {
        auto& set = std::get<DataSet>(data);
        uint32_t rest_size = set.size();
        for (auto& d : set) {
            --rest_size;
            acquire_data(d, rest_size, handle);
        }
        return StatusFlag::Success;
    }
    if (meta.stage == A_ReduceScatter::acquire_data1) {
        return acquire_data(std::get<DataPtr>(data), meta.rest_data, handle);
    }
    if (meta.stage == A_ReduceScatter::reduce_data) {
        return reduce_data(std::get<DataSet>(data), handle);
    }
    if (meta.stage == Public_ReduceScatter::agent_receive) {
        return scatter_data(std::get<DataSet>(data), handle);
    }
    if (meta.stage == A_ReduceScatter::scatter_data) {
        return send_data(std::get<DataSet>(data), handle);
    }
    if (meta.stage == A_ReduceScatter::finish_ack) {
        return close(handle);
    }
    TDCF_RAISE_ERROR(meta.stage error type)
}

StatusFlag DBTCluster::ReduceScatterAgent::proxy_event(const MetaData& meta,
                                                       Variant& data, Handle& handle) {
    return handle_event(meta, data, handle);
}

StatusFlag DBTCluster::ReduceScatterAgent::reduce_data(DataSet& dataset, Handle& handle) const {
    MetaData meta= create_meta();
    meta.stage = Public_ReduceScatter::agent_send;

    handle.create_processor_event(_other, meta, std::move(dataset));
    return StatusFlag::Success;
}

StatusFlag DBTCluster::ReduceScatterAgent::close(Handle& handle) const {
    MetaData meta = create_meta();
    meta.stage = Public_ReduceScatter::agent_finish;
    handle.create_processor_event(_other, meta, SerializablePtr());
    return StatusFlag::EventEnd;
}
