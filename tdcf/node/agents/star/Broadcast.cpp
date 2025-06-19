//
// Created by taganyer on 25-6-16.
//

#include <tdcf/base/Errors.hpp>
#include <tdcf/detail/NodeInformation.hpp>
#include <tdcf/node/agents/star/StarAgent.hpp>

using namespace tdcf;

StarAgent::Broadcast::Broadcast(ProcessingRulesPtr rp, const MetaData& meta) :
    EventProgress(ProgressType::NodeRoot, std::move(rp)), _root_meta(meta) {}

StatusFlag StarAgent::Broadcast::create(const MetaData& meta,
                                        ProcessingRulesPtr rp, NodeInformation& info) {
    assert(meta.operation_type == OperationType::Broadcast);
    assert(meta.stage == NodeAgentBroadcast::get_rule);

    MetaData new_meta(info.progress_events_version++, OperationType::Broadcast);
    new_meta.progress_type = ProgressType::Node;
    new_meta.stage = NodeAgentBroadcast::send_rule;
    auto [iter, success] = info.progress_events.emplace(
        new_meta, std::make_unique<Broadcast>(std::move(rp), meta));
    TDCF_CHECK_EXPR(success)

    auto& self = static_cast<Broadcast&>(*iter->second);
    if (!info.agent_factory) return StatusFlag::Success;

    StatusFlag flag = info.agent_factory->broadcast(self.rule, iter, info, &self._agent);
    if (flag != StatusFlag::Success || !self._agent) {
        info.progress_events.erase(iter);
        return flag;
    }
    return StatusFlag::Success;
}

StatusFlag StarAgent::Broadcast::handle_event(const MetaData& meta,
                                              Variant& data, NodeInformation& info) {
    assert(meta.operation_type == OperationType::Broadcast);
    if (!_agent) {
        assert(meta.stage == NodeAgentBroadcast::get_data);
        auto& data_ptr = std::get<DataPtr>(data);
        info.store_data(rule, data_ptr);
        return close(info);
    }
    if (meta.stage == NodeAgentBroadcast::get_data) {
        MetaData new_meta;
        new_meta.operation_type = OperationType::Broadcast;
        new_meta.stage = NodeAgentBroadcast::send_data;
        return _agent->store(new_meta, data, info);
    }
    /// 此时 _agent 指向的对象已销毁。
    if (meta.stage == NodeAgentBroadcast::finish_ack) {
        return close(info);
    }
    TDCF_RAISE_ERROR(error type)
}

StatusFlag StarAgent::Broadcast::close(NodeInformation& info) const {
    MetaData new_meta(_root_meta);
    new_meta.stage = NodeAgentBroadcast::finish;
    assert(info.root_id);
    StatusFlag flag = info.send_message(info.root_id, new_meta, nullptr);
    if (flag != StatusFlag::Success) return flag;
    return StatusFlag::EventEnd;
}
