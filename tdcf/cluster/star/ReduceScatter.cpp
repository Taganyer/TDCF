//
// Created by taganyer on 25-6-26.
//

#include <tdcf/base/Errors.hpp>
#include <tdcf/base/types/Star.hpp>
#include <tdcf/cluster/star/StarCluster.hpp>

using namespace tdcf;

using namespace tdcf::star;

StarCluster::ReduceScatter::ReduceScatter(ProgressType type, uint32_t version, ProcessingRulesPtr rp) :
    EventProgress(OperationType::ReduceScatter, type, version, std::move(rp)) {}

StatusFlag StarCluster::ReduceScatter::create(ProcessingRulesPtr rp, Handle& handle) {
    uint32_t version = handle.create_progress_version();
    auto iter = handle.create_progress(
        std::make_unique<ReduceScatter>(ProgressType::Root, version, std::move(rp)));

    auto& self = static_cast<ReduceScatter&>(*iter->second);
    self._self = iter;

    MetaData meta = self.create_meta();
    meta.stage = C_ReduceScatter::self_acquire_data;
    handle.acquire_data(iter, meta, self.rule);

    meta.stage = C_ReduceScatter::send_rule;
    uint32_t serial = 0;
    for (auto& id : handle.cluster_data<IdentityList>()) {
        meta.serial = ++serial;
        StatusFlag flag = handle.send_progress_message(version, id, meta, self.rule);
        TDCF_CHECK_SUCCESS(flag)
    }

    return StatusFlag::Success;
}

StatusFlag StarCluster::ReduceScatter::handle_event(const MetaData& meta,
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
        ++_respond;
        if (_respond == handle.cluster_data<IdentityList>().size()) {
            return StatusFlag::EventEnd;
        }
        return StatusFlag::Success;
    }
    TDCF_RAISE_ERROR(meta.stage error type)
}

StatusFlag StarCluster::ReduceScatter::acquire_data(DataPtr& data, uint32_t rest_size, Handle& handle) {
    _set.emplace_back(std::move(data));
    if (rest_size == 0) ++_received;
    if (_received == handle.cluster_data<IdentityList>().size() + 1) {
        MetaData new_meta = create_meta();
        new_meta.stage = C_ReduceScatter::reduce_data;
        handle.reduce_data(_self, new_meta, rule, std::move(_set));
    }
    return StatusFlag::Success;
}

StatusFlag StarCluster::ReduceScatter::scatter_data(DataSet& dataset, Handle& handle) const {
    MetaData meta = create_meta();
    meta.stage = C_ReduceScatter::scatter_data;
    handle.scatter_data(_self, meta, rule, handle.cluster_data<IdentityList>().size() + 1, std::move(dataset));
    return StatusFlag::Success;
}

StatusFlag StarCluster::ReduceScatter::send_data(DataSet& set, Handle& handle) const {
    uint32_t total = handle.cluster_data<IdentityList>().size() + 1;
    uint32_t patch = set.size() / total;

    MetaData meta = create_meta();
    meta.stage = C_ReduceScatter::send_data;
    meta.rest_data = patch;

    for (uint32_t i = 0; i < patch; ++i) {
        handle.store_data(rule, set[i]);
        --meta.rest_data;
        uint32_t j = i + patch;
        for (auto& id : handle.cluster_data<IdentityList>()) {
            StatusFlag flag = handle.send_progress_message(version, id, meta, std::move(set[j]));
            TDCF_CHECK_SUCCESS(flag)
            j += patch;
        }
    }

    return StatusFlag::Success;
}

StarCluster::ReduceScatterAgent::ReduceScatterAgent(uint32_t version, ProcessingRulesPtr rp, ProgressEventsMI iter) :
    ReduceScatter(ProgressType::NodeRoot, version, std::move(rp)), _other(iter) {}

StatusFlag StarCluster::ReduceScatterAgent::create(ProcessingRulesPtr rp, ProgressEventsMI other,
                                                   Handle& handle, EventProgressAgent **agent_ptr) {
    uint32_t version = handle.create_progress_version();
    auto iter = handle.create_progress(
        std::make_unique<ReduceScatterAgent>(version, std::move(rp), other));

    auto& self = static_cast<ReduceScatterAgent&>(*iter->second);
    *agent_ptr = &self;
    self._self = iter;

    MetaData meta = self.create_meta();
    meta.stage = A_ReduceScatter::self_acquire_data;
    handle.acquire_data(iter, meta, self.rule);

    meta.stage = A_ReduceScatter::send_rule;
    uint32_t serial = 0;
    for (auto& id : handle.cluster_data<IdentityList>()) {
        meta.serial = ++serial;
        StatusFlag flag = handle.send_progress_message(version, id, meta, self.rule);
        TDCF_CHECK_SUCCESS(flag)
    }

    return StatusFlag::Success;
}

StatusFlag StarCluster::ReduceScatterAgent::handle_event(const MetaData& meta,
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
        ++_respond;
        if (_respond == handle.cluster_data<IdentityList>().size()) {
            return close(handle);
        }
        return StatusFlag::Success;
    }
    TDCF_RAISE_ERROR(meta.stage error type)
}

StatusFlag StarCluster::ReduceScatterAgent::proxy_event(const MetaData& meta,
                                                        Variant& data, Handle& handle) {
    return handle_event(meta, data, handle);
}

StatusFlag StarCluster::ReduceScatterAgent::reduce_data(DataSet& dataset, Handle& handle) const {
    MetaData meta= create_meta();
    meta.stage = Public_ReduceScatter::agent_send;

    handle.create_processor_event(_other, meta, std::move(dataset));
    return StatusFlag::Success;
}

StatusFlag StarCluster::ReduceScatterAgent::close(Handle& handle) const {
    MetaData meta = create_meta();
    meta.stage = Public_ReduceScatter::agent_finish;
    handle.create_processor_event(_other, meta, SerializablePtr());
    return StatusFlag::EventEnd;
}
