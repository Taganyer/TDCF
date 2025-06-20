//
// Created by taganyer on 25-6-21.
//

#include <tdcf/base/Errors.hpp>
#include <tdcf/cluster/StarCluster.hpp>

using namespace tdcf;

StarCluster::Reduce::Reduce(ProgressType type, ProcessingRulesPtr rp) :
    EventProgress(type, std::move(rp)) {}

StatusFlag StarCluster::Reduce::create(ProcessingRulesPtr rp, NodeInformation& info) {
    MetaData meta(info.progress_events_version++, OperationType::Reduce);
    meta.progress_type = ProgressType::Root;

    auto [iter, success] = info.progress_events.emplace(
        meta, std::make_unique<Reduce>(ProgressType::Root, std::move(rp)));
    TDCF_CHECK_EXPR(success);

    auto& self = static_cast<Reduce&>(*iter->second);

    meta.stage = ClusterReduce::acquire_data;
    StatusFlag flag = info.acquire_data(iter, meta, self.rule);
    if (flag != StatusFlag::Success) {
        info.progress_events.erase(iter);
        return flag;
    }
    self._self = iter;

    meta.stage = ClusterReduce::send_rule;
    for (auto& id : info.identity_list) {
        flag = info.send_message(id, meta, self.rule);
        TDCF_CHECK_SUCCESS(flag)
    }
    return flag;
}

StatusFlag StarCluster::Reduce::handle_event(const MetaData& meta, Variant& data, NodeInformation& info) {
    assert(meta.operation_type == OperationType::Reduce);
    if (meta.stage == ClusterReduce::acquire_data) {
        return acquire_data(std::get<DataPtr>(data), info);
    }
    if (meta.stage == ClusterReduce::reduce_data) {
        info.store_data(rule, std::get<DataPtr>(data));
        rule->finish_callback();
        return StatusFlag::EventEnd;
    }
    TDCF_RAISE_ERROR(meta.stage error type)
}

StatusFlag StarCluster::Reduce::acquire_data(DataPtr& data, NodeInformation& info) {
    _set.push_back(std::move(data));
    if (_set.size() < info.cluster_size + 1) return StatusFlag::Success;
    MetaData meta(_self->first);
    meta.stage = ClusterReduce::reduce_data;
    StatusFlag flag = info.reduce_data(_self, meta, rule, _set);
    TDCF_CHECK_SUCCESS(flag)
    return flag;
}

StarCluster::ReduceAgent::ReduceAgent(ProcessingRulesPtr rp, ProgressEventsMI iter) :
    Reduce(ProgressType::NodeRoot, std::move(rp)), _other(iter) {}

StatusFlag StarCluster::ReduceAgent::create(ProcessingRulesPtr rp, ProgressEventsMI other,
                                            NodeInformation& info, EventProgressAgent **agent_ptr) {
    MetaData meta(info.progress_events_version++, OperationType::Reduce);
    meta.progress_type = ProgressType::NodeRoot;
    auto [iter, success] = info.progress_events.emplace(
        meta, std::make_unique<ReduceAgent>(std::move(rp), other));
    TDCF_CHECK_EXPR(success);

    auto& self = static_cast<ReduceAgent&>(*iter->second);

    meta.stage = AgentReduce::acquire_data;
    StatusFlag flag = info.acquire_data(iter, meta, self.rule);
    if (flag != StatusFlag::Success) {
        info.progress_events.erase(iter);
        return flag;
    }

    meta.stage = AgentReduce::send_rule;
    for (auto& id : info.identity_list) {
        flag = info.send_message(id, meta, self.rule);
        TDCF_CHECK_SUCCESS(flag)
    }

    *agent_ptr = &self;
    self._self = iter;
    return StatusFlag::Success;
}

StatusFlag StarCluster::ReduceAgent::handle_event(const MetaData& meta, Variant& data, NodeInformation& info) {
    assert(meta.operation_type == OperationType::Reduce);
    if (meta.stage == AgentReduce::acquire_data) {
        return acquire_data(std::get<DataPtr>(data), info);
    }
    if (meta.stage == AgentReduce::reduce_data) {
        return close(std::get<DataPtr>(data), info);
    }
    TDCF_RAISE_ERROR(meta.stage error type)
}

StatusFlag StarCluster::ReduceAgent::store(const MetaData& meta, Variant& data, NodeInformation& info) {
    return handle_event(meta, data, info);
}

StatusFlag StarCluster::ReduceAgent::close(DataPtr& data, NodeInformation& info) const {
    MetaData meta(_other->first);
    meta.stage = AgentReduce::send_data;
    info.processed_queue.emplace(_other, meta, static_cast<SerializablePtr&&>(std::move(data)));
    return StatusFlag::EventEnd;
}
