//
// Created by taganyer on 25-6-26.
//

#include <tdcf/base/Errors.hpp>
#include <tdcf/cluster/StarCluster.hpp>

using namespace tdcf;

StarCluster::ReduceScatter::ReduceScatter(ProgressType type, uint32_t version, ProcessingRulesPtr rp) :
    EventProgress(OperationType::ReduceScatter, type, version, std::move(rp)) {}

StatusFlag StarCluster::ReduceScatter::create(ProcessingRulesPtr rp, Handle& handle) {
    uint32_t version = handle.create_conversation_version();
    auto iter = handle.create_progress(
        std::make_unique<ReduceScatter>(ProgressType::Root, version, std::move(rp)));

    auto& self = static_cast<ReduceScatter&>(*iter->second);
    self._self = iter;
    self._set.resize(handle.cluster_size() + 1);
    self._get.resize(handle.cluster_size() + 1);

    MetaData meta = self.create_meta();
    meta.stage = ClusterReduceScatter::acquire_data;
    handle.acquire_data(iter, meta, self.rule);

    meta.stage = ClusterReduceScatter::send_rule;
    uint32_t serial = 0;
    for (auto& id : handle.identities) {
        meta.serial = ++serial;
        StatusFlag flag = handle.start_progress_message(version, id, meta, self.rule);
        TDCF_CHECK_SUCCESS(flag)
    }

    return StatusFlag::Success;
}

StatusFlag StarCluster::ReduceScatter::handle_event(const MetaData& meta,
                                                    Variant& data, Handle& handle) {
    assert(meta.operation_type == OperationType::ReduceScatter);
    if (meta.stage == ClusterAllReduce::acquire_data) {
        return acquire_data(meta, std::get<DataPtr>(data), handle);
    }
    if (meta.stage == ClusterAllReduce::reduce_data) {
        return scatter_data(std::get<DataPtr>(data), handle);
    }
    if (meta.stage == ClusterReduceScatter::scatter_data) {
        return send_data(std::get<DataSet>(data), handle);
    }
    if (meta.stage == ClusterReduceScatter::finish_ack) {
        ++_respond;
        if (_respond == handle.cluster_size()) {
            rule->finish_callback();
            return StatusFlag::EventEnd;
        }
        return StatusFlag::Success;
    }
    TDCF_RAISE_ERROR(meta.stage error type)
}

StatusFlag StarCluster::ReduceScatter::acquire_data(const MetaData& meta,
                                                    DataPtr& data, Handle& handle) {
    assert(!_get[meta.serial]);
    _set[meta.serial] = std::move(data);
    _get[meta.serial] = true;
    ++_received;
    if (_received == handle.cluster_size() + 1) {
        MetaData new_meta = create_meta();
        new_meta.stage = ClusterReduceScatter::reduce_data;
        handle.reduce_data(_self, new_meta, rule, _set);
    }
    return StatusFlag::Success;
}

StatusFlag StarCluster::ReduceScatter::scatter_data(DataPtr& data, Handle& handle) {
    _set.clear();
    MetaData meta = create_meta();
    meta.stage = ClusterReduceScatter::scatter_data;
    handle.scatter_data(_self, meta, rule, handle.cluster_size() + 1, data);
    return StatusFlag::Success;
}

StatusFlag StarCluster::ReduceScatter::send_data(DataSet& set, Handle& handle) const {
    assert(set.size() == handle.cluster_size() + 1);
    MetaData meta = create_meta();
    meta.stage = ClusterReduceScatter::send_data;
    handle.store_data(rule, set[0]);
    uint32_t serial = 1;
    for (auto &id : handle.identities) {
        StatusFlag flag = handle.send_progress_message(version, id, meta, std::move(set[serial]));
        TDCF_CHECK_SUCCESS(flag)
        ++serial;
    }
    return StatusFlag::Success;
}

StarCluster::ReduceScatterAgent::ReduceScatterAgent(uint32_t version, ProcessingRulesPtr rp, ProgressEventsMI iter) :
    ReduceScatter(ProgressType::NodeRoot, version, std::move(rp)), _other(iter) {}

StatusFlag StarCluster::ReduceScatterAgent::create(ProcessingRulesPtr rp, ProgressEventsMI other,
                                                   Handle& handle, EventProgressAgent **agent_ptr) {
    uint32_t version = handle.create_conversation_version();
    auto iter = handle.create_progress(
        std::make_unique<ReduceScatterAgent>(version, std::move(rp), other));

    auto& self = static_cast<ReduceScatterAgent&>(*iter->second);
    *agent_ptr = &self;
    self._self = iter;
    self._set.resize(handle.cluster_size() + 1);
    self._get.resize(handle.cluster_size() + 1);

    MetaData meta = self.create_meta();
    meta.stage = AgentReduceScatter::acquire_data1;
    handle.acquire_data(iter, meta, self.rule);

    meta.stage = AgentReduceScatter::send_rule;
    uint32_t serial = 0;
    for (auto& id : handle.identities) {
        meta.serial = ++serial;
        StatusFlag flag = handle.start_progress_message(version, id, meta, self.rule);
        TDCF_CHECK_SUCCESS(flag)
    }

    return StatusFlag::Success;
}

StatusFlag StarCluster::ReduceScatterAgent::handle_event(const MetaData& meta,
                                                         Variant& data, Handle& handle) {
    assert(meta.operation_type == OperationType::ReduceScatter);
    if (meta.stage == AgentReduceScatter::acquire_data1) {
        return acquire_data(meta, std::get<DataPtr>(data), handle);
    }
    if (meta.stage == AgentReduceScatter::reduce_data) {
        return reduce_data(std::get<DataPtr>(data), handle);
    }
    if (meta.stage == AgentReduceScatter::acquire_data2) {
        return scatter_data(std::get<DataPtr>(data), handle);
    }
    if (meta.stage == AgentReduceScatter::scatter_data) {
        return send_data(std::get<DataSet>(data), handle);
    }
    if (meta.stage == AgentReduceScatter::finish_ack) {
        ++_respond;
        if (_respond == handle.cluster_size()) {
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

StatusFlag StarCluster::ReduceScatterAgent::reduce_data(DataPtr& data, Handle& handle) {
    _set.clear();
    MetaData meta= create_meta();
    meta.stage = AgentReduceScatter::send_data1;
    handle.create_processor_event(_other, meta, std::static_pointer_cast<Serializable>(data));
    return StatusFlag::Success;
}

StatusFlag StarCluster::ReduceScatterAgent::close(Handle& handle) const {
    MetaData meta = create_meta();
    meta.stage = AgentReduceScatter::finish;
    handle.create_processor_event(_other, meta, SerializablePtr());
    return StatusFlag::EventEnd;
}
