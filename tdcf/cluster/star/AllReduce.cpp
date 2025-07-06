//
// Created by taganyer on 25-6-22.
//

#include <tdcf/base/Errors.hpp>
#include <tdcf/cluster/star/StarCluster.hpp>

using namespace tdcf;

StarCluster::AllReduce::AllReduce(ProgressType type, uint32_t version, ProcessingRulesPtr rp) :
    EventProgress(OperationType::AllReduce, type, version, std::move(rp)) {}

StatusFlag StarCluster::AllReduce::create(ProcessingRulesPtr rp, Handle& handle) {
    uint32_t version = handle.create_conversation_version();
    auto iter = handle.create_progress(
        std::make_unique<AllReduce>(ProgressType::Root, version, std::move(rp)));

    auto& self = static_cast<AllReduce&>(*iter->second);
    self._self = iter;
    self._set.resize(handle.cluster_data<IdentityList>().size() + 1);
    self._get.resize(handle.cluster_data<IdentityList>().size() + 1);

    MetaData meta = self.create_meta();
    meta.stage = ClusterAllReduce::acquire_data;
    handle.acquire_data(iter, meta, self.rule);

    meta.stage = ClusterAllReduce::send_rule;
    uint32_t serial = 0;
    for (auto& id : handle.cluster_data<IdentityList>()) {
        meta.serial = ++serial;
        StatusFlag flag = handle.start_progress_message(version, id, meta, self.rule);
        TDCF_CHECK_SUCCESS(flag)
    }

    return StatusFlag::Success;
}

StatusFlag StarCluster::AllReduce::handle_event(const MetaData& meta,
                                                Variant& data, Handle& handle) {
    assert(meta.operation_type == OperationType::AllReduce);
    if (meta.stage == ClusterAllReduce::acquire_data) {
        return acquire_data(meta, std::get<DataPtr>(data), handle);
    }
    if (meta.stage == ClusterAllReduce::reduce_data) {
        return send_data(std::get<DataPtr>(data), handle);
    }
    if (meta.stage == ClusterAllReduce::finish_ack) {
        ++_respond;
        if (_respond == handle.cluster_data<IdentityList>().size()) {
            rule->finish_callback();
            return StatusFlag::EventEnd;
        }
        return StatusFlag::Success;
    }
    TDCF_RAISE_ERROR(meta.stage error type)
}

StatusFlag StarCluster::AllReduce::acquire_data(const MetaData& meta,
                                                DataPtr& data, Handle& handle) {
    assert(!_get[meta.serial]);
    _set[meta.serial] = std::move(data);
    _get[meta.serial] = true;
    ++_received;
    if (_received == handle.cluster_data<IdentityList>().size() + 1) {
        MetaData new_meta = create_meta();
        new_meta.stage = ClusterAllReduce::reduce_data;
        handle.reduce_data(_self, new_meta, rule, _set);
    }
    return StatusFlag::Success;
}

StatusFlag StarCluster::AllReduce::send_data(DataPtr& data, Handle& handle) {
    _set.clear();
    MetaData meta = create_meta();
    meta.stage = ClusterAllReduce::send_data;
    handle.store_data(rule, data);
    for (auto& id : handle.cluster_data<IdentityList>()) {
        StatusFlag flag = handle.send_progress_message(version, id, meta, data);
        TDCF_CHECK_SUCCESS(flag)
    }
    return StatusFlag::Success;
}

StarCluster::AllReduceAgent::AllReduceAgent(uint32_t version, ProcessingRulesPtr rp, ProgressEventsMI iter) :
    AllReduce(ProgressType::NodeRoot, version, std::move(rp)), _other(iter) {}

StatusFlag StarCluster::AllReduceAgent::create(ProcessingRulesPtr rp, ProgressEventsMI other,
                                               Handle& handle, EventProgressAgent **agent_ptr) {
    uint32_t version = handle.create_conversation_version();
    auto iter = handle.create_progress(
        std::make_unique<AllReduceAgent>(version, std::move(rp), other));

    auto& self = static_cast<AllReduceAgent&>(*iter->second);
    *agent_ptr = &self;
    self._self = iter;
    self._set.resize(handle.cluster_data<IdentityList>().size() + 1);
    self._get.resize(handle.cluster_data<IdentityList>().size() + 1);

    MetaData meta = self.create_meta();
    meta.stage = AgentAllReduce::acquire_data1;
    handle.acquire_data(iter, meta, self.rule);

    meta.stage = AgentAllReduce::send_rule;
    uint32_t serial = 0;
    for (auto& id : handle.cluster_data<IdentityList>()) {
        meta.serial = ++serial;
        StatusFlag flag = handle.start_progress_message(version, id, meta, self.rule);
        TDCF_CHECK_SUCCESS(flag)
    }

    return StatusFlag::Success;
}

StatusFlag StarCluster::AllReduceAgent::handle_event(const MetaData& meta,
                                                     Variant& data, Handle& handle) {
    assert(meta.operation_type == OperationType::AllReduce);
    if (meta.stage == AgentAllReduce::acquire_data1) {
        return acquire_data(meta, std::get<DataPtr>(data), handle);
    }
    if (meta.stage == AgentAllReduce::reduce_data) {
        return reduce_data(std::get<DataPtr>(data), handle);
    }
    if (meta.stage == AgentAllReduce::acquire_data2) {
        return send_data(std::get<DataPtr>(data), handle);
    }
    if (meta.stage == AgentAllReduce::finish_ack) {
        ++_respond;
        if (_respond == handle.cluster_data<IdentityList>().size()) {
            return close(handle);
        }
        return StatusFlag::Success;
    }
    TDCF_RAISE_ERROR(meta.stage error type)
}

StatusFlag StarCluster::AllReduceAgent::proxy_event(const MetaData& meta,
                                              Variant& data, Handle& handle) {
    return handle_event(meta, data, handle);
}

StatusFlag StarCluster::AllReduceAgent::reduce_data(DataPtr& data, Handle& handle) {
    _set.clear();
    MetaData meta = create_meta();
    meta.stage = AgentAllReduce::send_data;
    handle.create_processor_event(_other, meta, std::static_pointer_cast<Serializable>(data));
    return StatusFlag::Success;
}

StatusFlag StarCluster::AllReduceAgent::close(Handle& handle) const {
    MetaData meta = create_meta();
    meta.stage = AgentAllReduce::finish;
    handle.create_processor_event(_other, meta, nullptr);
    return StatusFlag::EventEnd;
}
