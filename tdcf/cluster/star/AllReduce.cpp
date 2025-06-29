//
// Created by taganyer on 25-6-22.
//

#include <tdcf/base/Errors.hpp>
#include <tdcf/cluster/StarCluster.hpp>

using namespace tdcf;

StarCluster::AllReduce::AllReduce(ProgressType type, ProcessingRulesPtr rp) :
    EventProgress(type, std::move(rp)) {}

StatusFlag StarCluster::AllReduce::create(ProcessingRulesPtr rp, Handle& info) {
    MetaData meta(info.get_version(), OperationType::AllReduce);
    meta.progress_type = ProgressType::Root;
    meta.data4[0] = info.cluster_size() + 1;

    auto [iter, success] = info.progress_events.emplace(
        meta, std::make_unique<AllReduce>(ProgressType::Root, std::move(rp)));
    TDCF_CHECK_EXPR(success)

    auto& self = static_cast<AllReduce&>(*iter->second);
    self._self = iter;
    self._set.resize(info.cluster_size());
    self._get.resize(info.cluster_size());

    meta.stage = ClusterAllReduce::acquire_data;
    info.acquire_data(iter, meta, self.rule);

    meta.stage = ClusterAllReduce::send_rule;
    unsigned serial = 1;
    for (auto& id : info.identity_list) {
        meta.serial = serial++;
        StatusFlag flag = info.send_message(id, meta, self.rule);
        TDCF_CHECK_SUCCESS(flag)
    }

    return StatusFlag::Success;
}

StatusFlag StarCluster::AllReduce::handle_event(const MetaData& meta,
                                                Variant& data, Handle& info) {
    assert(meta.operation_type == OperationType::AllReduce);
    if (meta.stage == ClusterAllReduce::acquire_data) {
        return acquire_data(meta, std::get<DataPtr>(data), info);
    }
    if (meta.stage == ClusterAllReduce::reduce_data) {
        return send_data(std::get<DataPtr>(data), info);
    }
    if (meta.stage == ClusterAllReduce::finish_ack) {
        ++_respond;
        if (_respond == info.cluster_size()) {
            rule->finish_callback();
            return StatusFlag::EventEnd;
        }
        return StatusFlag::Success;
    }
    TDCF_RAISE_ERROR(meta.stage error type)
}

StatusFlag StarCluster::AllReduce::acquire_data(const MetaData& meta,
                                                DataPtr& data, Handle& info) {
    assert(!_get[meta.serial]);
    _set[meta.serial] = std::move(data);
    _get[meta.serial] = true;
    ++_received;
    if (_received == info.cluster_size() + 1) {
        MetaData new_meta(_self->first);
        new_meta.stage = ClusterAllReduce::reduce_data;
        info.reduce_data(_self, new_meta, rule, _set);
    }
    return StatusFlag::Success;
}

StatusFlag StarCluster::AllReduce::send_data(DataPtr& data, Handle& info) {
    _set.clear();
    MetaData meta(_self->first);
    meta.stage = ClusterAllReduce::send_data;
    info.store_data(rule, data);
    for (auto& id : info.identity_list) {
        StatusFlag flag = info.send_message(id, meta, data);
        TDCF_CHECK_SUCCESS(flag)
    }
    return StatusFlag::Success;
}

StarCluster::AllReduceAgent::AllReduceAgent(ProcessingRulesPtr rp, ProgressEventsMI iter) :
    AllReduce(ProgressType::NodeRoot, std::move(rp)), _other(iter) {}

StatusFlag StarCluster::AllReduceAgent::create(ProcessingRulesPtr rp, ProgressEventsMI other,
                                               Handle& info, EventProgressAgent **agent_ptr) {
    MetaData meta(info.get_version(), OperationType::AllReduce);
    meta.progress_type = ProgressType::NodeRoot;
    auto [iter, success] = info.progress_events.emplace(
        meta, std::make_unique<AllReduceAgent>(std::move(rp), other));
    TDCF_CHECK_EXPR(success)

    auto& self = static_cast<AllReduceAgent&>(*iter->second);
    *agent_ptr = &self;
    self._self = iter;
    self._set.resize(info.cluster_size());
    self._get.resize(info.cluster_size());

    meta.stage = AgentAllReduce::acquire_data1;
    info.acquire_data(iter, meta, self.rule);

    meta.stage = AgentAllReduce::send_rule;
    for (auto& id : info.identity_list) {
        StatusFlag flag = info.send_message(id, meta, self.rule);
        TDCF_CHECK_SUCCESS(flag)
    }

    return StatusFlag::Success;
}

StatusFlag StarCluster::AllReduceAgent::handle_event(const MetaData& meta,
                                                     Variant& data, Handle& info) {
    assert(meta.operation_type == OperationType::AllReduce);
    if (meta.stage == AgentAllReduce::acquire_data1) {
        return acquire_data(meta, std::get<DataPtr>(data), info);
    }
    if (meta.stage == AgentAllReduce::reduce_data) {
        return reduce_data(std::get<DataPtr>(data), info);
    }
    if (meta.stage == AgentAllReduce::acquire_data2) {
        return send_data(std::get<DataPtr>(data), info);
    }
    if (meta.stage == AgentAllReduce::finish_ack) {
        ++_received;
        if (_received == info.cluster_size()) {
            return close(info);
        }
        return StatusFlag::Success;
    }
    TDCF_RAISE_ERROR(meta.stage error type)
}

StatusFlag StarCluster::AllReduceAgent::proxy_event(const MetaData& meta,
                                              Variant& data, Handle& info) {
    return handle_event(meta, data, info);
}

StatusFlag StarCluster::AllReduceAgent::reduce_data(DataPtr& data, Handle& info) {
    _set.clear();
    MetaData meta(_other->first);
    meta.stage = AgentAllReduce::send_data;
    info.processed_queue.emplace(_other, meta, std::static_pointer_cast<Serializable>(data));
    return StatusFlag::Success;
}

StatusFlag StarCluster::AllReduceAgent::close(Handle& info) {
    MetaData meta(_other->first);
    meta.stage = AgentAllReduce::finish;
    info.processed_queue.emplace(_other, meta, SerializablePtr());
    return StatusFlag::EventEnd;
}
