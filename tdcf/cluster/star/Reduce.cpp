//
// Created by taganyer on 25-6-21.
//

#include <tdcf/base/Errors.hpp>
#include <tdcf/cluster/StarCluster.hpp>

using namespace tdcf;

StarCluster::Reduce::Reduce(ProgressType type, ProcessingRulesPtr rp) :
    EventProgress(type, std::move(rp)) {}

StatusFlag StarCluster::Reduce::create(ProcessingRulesPtr rp, NodeInformation& info) {
    MetaData meta(info.get_version(), OperationType::Reduce);
    meta.progress_type = ProgressType::Root;

    auto [iter, success] = info.progress_events.emplace(
        meta, std::make_unique<Reduce>(ProgressType::Root, std::move(rp)));
    TDCF_CHECK_EXPR(success);

    auto& self = static_cast<Reduce&>(*iter->second);
    self._self = iter;
    self._set.reserve(info.cluster_size() + 1);

    meta.stage = ClusterReduce::acquire_data;
    info.acquire_data(iter, meta, self.rule);

    meta.stage = ClusterReduce::send_rule;
    for (auto& id : info.identity_list) {
        StatusFlag flag = info.send_message(id, meta, self.rule);
        TDCF_CHECK_SUCCESS(flag)
    }

    return StatusFlag::Success;
}

StatusFlag StarCluster::Reduce::handle_event(const MetaData& meta,
                                             Variant& data, NodeInformation& info) {
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
    if (_set.size() < info.cluster_size() + 1) return StatusFlag::Success;
    MetaData meta(_self->first);
    meta.stage = ClusterReduce::reduce_data;
    info.reduce_data(_self, meta, rule, _set);
    return StatusFlag::Success;
}

StarCluster::ReduceAgent::ReduceAgent(ProcessingRulesPtr rp, ProgressEventsMI iter) :
    Reduce(ProgressType::NodeRoot, std::move(rp)), _other(iter) {}

StatusFlag StarCluster::ReduceAgent::create(ProcessingRulesPtr rp, ProgressEventsMI other,
                                            NodeInformation& info, EventProgressAgent **agent_ptr) {
    MetaData meta(info.get_version(), OperationType::Reduce);
    meta.progress_type = ProgressType::NodeRoot;
    auto [iter, success] = info.progress_events.emplace(
        meta, std::make_unique<ReduceAgent>(std::move(rp), other));
    TDCF_CHECK_EXPR(success);

    auto& self = static_cast<ReduceAgent&>(*iter->second);
    *agent_ptr = &self;
    self._self = iter;

    meta.stage = AgentReduce::acquire_data;
    info.acquire_data(iter, meta, self.rule);

    meta.stage = AgentReduce::send_rule;
    for (auto& id : info.identity_list) {
        StatusFlag flag = info.send_message(id, meta, self.rule);
        TDCF_CHECK_SUCCESS(flag)
    }

    return StatusFlag::Success;
}

StatusFlag StarCluster::ReduceAgent::handle_event(const MetaData& meta,
                                                  Variant& data, NodeInformation& info) {
    assert(meta.operation_type == OperationType::Reduce);
    if (meta.stage == AgentReduce::acquire_data) {
        return acquire_data(std::get<DataPtr>(data), info);
    }
    if (meta.stage == AgentReduce::reduce_data) {
        return close(std::get<DataPtr>(data), info);
    }
    TDCF_RAISE_ERROR(meta.stage error type)
}

StatusFlag StarCluster::ReduceAgent::proxy_event(const MetaData& meta,
                                           Variant& data, NodeInformation& info) {
    return handle_event(meta, data, info);
}

StatusFlag StarCluster::ReduceAgent::close(DataPtr& data, NodeInformation& info) const {
    MetaData meta(_other->first);
    meta.stage = AgentReduce::send_data;
    info.processed_queue.emplace(_other, meta, std::static_pointer_cast<Serializable>(data));
    return StatusFlag::EventEnd;
}
