//
// Created by taganyer on 25-6-20.
//

#include <tdcf/base/Errors.hpp>
#include <tdcf/detail/NodeInformation.hpp>
#include <tdcf/node/agents/star/StarAgent.hpp>

using namespace tdcf;

StarAgent::Scatter::Scatter(ProcessingRulesPtr rp, const MetaData& meta) :
    EventProgress(ProgressType::NodeRoot, std::move(rp)), _root_meta(meta) {}

StatusFlag StarAgent::Scatter::create(const MetaData& meta,
                                      ProcessingRulesPtr rp, NodeInformation& info) {
    assert(meta.operation_type == OperationType::Scatter);
    assert(meta.stage == NodeAgentScatter::get_rule);

    MetaData new_meta(info.get_version(), OperationType::Scatter);
    new_meta.progress_type = ProgressType::Node;
    auto [iter, success] = info.progress_events.emplace(
        new_meta, std::make_unique<Scatter>(std::move(rp), meta));
    TDCF_CHECK_EXPR(success)

    if (!info.agent_factory) return StatusFlag::Success;

    auto& self = static_cast<Scatter&>(*iter->second);
    self._self = iter;

    StatusFlag flag = info.agent_factory->scatter(self.rule, iter, info, &self._agent);
    if (flag != StatusFlag::Success || !self._agent) {
        info.progress_events.erase(iter);
        return flag;
    }
    return StatusFlag::Success;
}

StatusFlag StarAgent::Scatter::handle_event(const MetaData& meta,
                                            Variant& data, NodeInformation& info) {
    assert(meta.operation_type == OperationType::Scatter);
    if (!_agent) {
        assert(meta.stage == NodeAgentScatter::get_data);
        info.store_data(rule, std::get<DataPtr>(data));
        return close(info);
    }
    if (meta.stage == NodeAgentScatter::get_data) {
        return scatter_data(std::get<DataPtr>(data), info);
    }
    if (meta.stage == NodeAgentScatter::scatter_data) {
        return agent_store(data, info);
    }
    /// 此时 _agent 指向的对象已销毁。
    if (meta.stage == NodeAgentScatter::finish_ack) {
        return close(info);
    }
    TDCF_RAISE_ERROR(meta.stage error type)
}

StatusFlag StarAgent::Scatter::scatter_data(DataPtr& data, NodeInformation& info) const {
    MetaData meta(_self->first);
    meta.stage = NodeAgentScatter::scatter_data;
    info.scatter_data(_self, meta, rule, info.cluster_size() + 1, data);
    return StatusFlag::Success;
}

StatusFlag StarAgent::Scatter::agent_store(Variant& data, NodeInformation& info) const {
    auto& set = std::get<DataSet>(data);
    assert(set.size() == info.cluster_size() + 1);

    MetaData meta;
    meta.operation_type = OperationType::Scatter;
    meta.stage = NodeAgentScatter::send_data;
    return _agent->proxy_event(meta, data, info);
}

StatusFlag StarAgent::Scatter::close(NodeInformation& info) const {
    MetaData meta(_root_meta);
    meta.stage = NodeAgentScatter::finish;
    assert(info.root_id());
    StatusFlag flag = info.send_message(info.root_id(), meta, nullptr);
    if (flag != StatusFlag::Success) return flag;
    return StatusFlag::EventEnd;
}
