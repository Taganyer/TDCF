//
// Created by taganyer on 25-6-26.
//

#include <tdcf/base/Errors.hpp>
#include <tdcf/cluster/StarCluster.hpp>

using namespace tdcf;

StarCluster::ReduceScatter::ReduceScatter(ProgressType type, ProcessingRulesPtr rp) :
    EventProgress(type, std::move(rp)) {}

StatusFlag StarCluster::ReduceScatter::create(ProcessingRulesPtr rp, Handle& info) {
    MetaData meta(info.get_version(), OperationType::ReduceScatter);
    meta.progress_type = ProgressType::Root;
    meta.data4[0] = info.cluster_size() + 1;

    auto [iter, success] = info.progress_events.emplace(
        meta, std::make_unique<ReduceScatter>(ProgressType::Root, std::move(rp)));
    TDCF_CHECK_EXPR(success)

    auto& self = static_cast<ReduceScatter&>(*iter->second);
    self._self = iter;
    self._set.resize(info.cluster_size());
    self._get.resize(info.cluster_size());

    meta.stage = ClusterReduceScatter::acquire_data;
    info.acquire_data(iter, meta, self.rule);

    meta.stage = ClusterReduceScatter::send_rule;
    unsigned serial = 1;
    for (auto& id : info.identity_list) {
        meta.serial = serial++;
        StatusFlag flag = info.send_message(id, meta, self.rule);
        TDCF_CHECK_SUCCESS(flag)
    }

    return StatusFlag::Success;
}

StatusFlag StarCluster::ReduceScatter::handle_event(const MetaData& meta,
                                                    Variant& data, Handle& info) {
    assert(meta.operation_type == OperationType::ReduceScatter);
    if (meta.stage == ClusterAllReduce::acquire_data) {
        return acquire_data(meta, std::get<DataPtr>(data), info);
    }
    if (meta.stage == ClusterAllReduce::reduce_data) {
        return scatter_data(std::get<DataPtr>(data), info);
    }
    if (meta.stage == ClusterReduceScatter::scatter_data) {
        return send_data(std::get<DataSet>(data), info);
    }
    if (meta.stage == ClusterReduceScatter::finish_ack) {
        ++_respond;
        if (_respond == info.cluster_size()) {
            rule->finish_callback();
            return StatusFlag::EventEnd;
        }
        return StatusFlag::Success;
    }
    TDCF_RAISE_ERROR(meta.stage error type)
}

StatusFlag StarCluster::ReduceScatter::acquire_data(const MetaData& meta,
                                                    DataPtr& data, Handle& info) {
    assert(!_get[meta.serial]);
    _set[meta.serial] = std::move(data);
    _get[meta.serial] = true;
    ++_received;
    if (_received == info.cluster_size() + 1) {
        MetaData new_meta(_self->first);
        new_meta.stage = ClusterReduceScatter::reduce_data;
        info.reduce_data(_self, new_meta, rule, _set);
    }
    return StatusFlag::Success;
}

StatusFlag StarCluster::ReduceScatter::scatter_data(DataPtr& data, Handle& info) {
    _set.clear();
    MetaData meta(_self->first);
    meta.stage = ClusterReduceScatter::scatter_data;
    info.scatter_data(_self, meta, rule, info.cluster_size() + 1, data);
    return StatusFlag::Success;
}

StatusFlag StarCluster::ReduceScatter::send_data(DataSet& set, Handle& info) const {
    assert(set.size() == info.cluster_size() + 1);
    MetaData meta(_self->first);
    meta.stage = ClusterReduceScatter::send_data;
    info.store_data(rule, set[0]);
    for (unsigned i = 0; i < info.cluster_size(); ++i) {
        StatusFlag flag = info.send_message(info.identity_list[i], meta, std::move(set[i + 1]));
        TDCF_CHECK_SUCCESS(flag)
    }
    return StatusFlag::Success;
}

StarCluster::ReduceScatterAgent::ReduceScatterAgent(ProcessingRulesPtr rp, ProgressEventsMI iter) :
    ReduceScatter(ProgressType::NodeRoot, std::move(rp)), _other(iter) {}

StatusFlag StarCluster::ReduceScatterAgent::create(ProcessingRulesPtr rp, ProgressEventsMI other,
                                                   Handle& info, EventProgressAgent **agent_ptr) {
    MetaData meta(info.get_version(), OperationType::ReduceScatter);
    meta.progress_type = ProgressType::NodeRoot;
    auto [iter, success] = info.progress_events.emplace(
        meta, std::make_unique<ReduceScatterAgent>(std::move(rp), other));
    TDCF_CHECK_EXPR(success)

    auto& self = static_cast<ReduceScatterAgent&>(*iter->second);
    *agent_ptr = &self;
    self._self = iter;
    self._set.resize(info.cluster_size());
    self._get.resize(info.cluster_size());

    meta.stage = AgentReduceScatter::acquire_data1;
    info.acquire_data(iter, meta, self.rule);

    meta.stage = AgentReduceScatter::send_rule;
    for (auto& id : info.identity_list) {
        StatusFlag flag = info.send_message(id, meta, self.rule);
        TDCF_CHECK_SUCCESS(flag)
    }

    return StatusFlag::Success;
}

StatusFlag StarCluster::ReduceScatterAgent::handle_event(const MetaData& meta,
                                                         Variant& data, Handle& info) {
    assert(meta.operation_type == OperationType::ReduceScatter);
    if (meta.stage == AgentReduceScatter::acquire_data1) {
        return acquire_data(meta, std::get<DataPtr>(data), info);
    }
    if (meta.stage == AgentReduceScatter::reduce_data) {
        return reduce_data(std::get<DataPtr>(data), info);
    }
    if (meta.stage == AgentReduceScatter::acquire_data2) {
        return scatter_data(std::get<DataPtr>(data), info);
    }
    if (meta.stage == AgentReduceScatter::scatter_data) {
        return send_data(std::get<DataSet>(data), info);
    }
    if (meta.stage == AgentReduceScatter::finish_ack) {
        ++_respond;
        if (_respond == info.cluster_size()) {
            return close(info);
        }
        return StatusFlag::Success;
    }
    TDCF_RAISE_ERROR(meta.stage error type)
}

StatusFlag StarCluster::ReduceScatterAgent::proxy_event(const MetaData& meta,
                                                        Variant& data, Handle& info) {
    return handle_event(meta, data, info);
}

StatusFlag StarCluster::ReduceScatterAgent::reduce_data(DataPtr& data, Handle& info) {
    _set.clear();
    MetaData meta(_other->first);
    meta.stage = AgentReduceScatter::send_data1;
    info.processed_queue.emplace(_other, meta, std::static_pointer_cast<Serializable>(data));
    return StatusFlag::Success;
}

StatusFlag StarCluster::ReduceScatterAgent::close(Handle& info) {
    MetaData meta(_other->first);
    meta.stage = AgentReduceScatter::finish;
    info.processed_queue.emplace(_other, meta, SerializablePtr());
    return StatusFlag::EventEnd;
}
