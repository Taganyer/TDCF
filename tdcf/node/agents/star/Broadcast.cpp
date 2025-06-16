//
// Created by taganyer on 25-6-16.
//

#include <tdcf/base/Errors.hpp>
#include <tdcf/detail/NodeInformation.hpp>
#include <tdcf/detail/OperationInterpreter.hpp>
#include <tdcf/node/agents/star/StarAgent.hpp>

using namespace tdcf;


StatusFlag StarAgent::Broadcast::create(const MetaData& meta,
                                        ProcessingRulesPtr rp, NodeInformation& info) {
    assert(meta.type == MetaDataTypes::Broadcast);
    MetaData new_meta(info.progress_events_version++, MetaDataTypes::Broadcast);
    auto [iter, success] = info.progress_events.emplace(
        new_meta, std::make_unique<Broadcast>(std::move(rp), meta));
    TDCF_CHECK_EXPR(success)

    auto& self = static_cast<Broadcast&>(*iter->second);
    StatusFlag flag = info.agent_factory->broadcast(self.rule, iter, info, &self._agent);
    if (flag != StatusFlag::Success || !self._agent) {
        info.progress_events.erase(iter);
        return flag;
    }
    return StatusFlag::Success;
}

StatusFlag StarAgent::Broadcast::handle_event(const MetaData& meta,
                                              Variant *data, NodeInformation& node_info) {
    assert(meta.type == MetaDataTypes::Broadcast);
    if (meta.stage == NodeAgentBroadcast::acquire) {
        MetaData new_meta;
        new_meta.type = MetaDataTypes::Broadcast;
        new_meta.stage = NodeAgentBroadcast::send;
        return _agent->store(new_meta, data, node_info);
    }
    if (meta.stage == NodeAgentBroadcast::finish_ack) {
        MetaData new_meta(old_meta);
        new_meta.stage = NodeAgentBroadcast::finish;
        assert(node_info.root_id);
        return node_info.send_message(node_info.root_id, new_meta, nullptr);
    }
    TDCF_RAISE_ERROR(error type)
}

StarAgent::Broadcast::Broadcast(ProcessingRulesPtr rp, const MetaData& meta) :
    EventProgress(EventType::CTNBroadcast, std::move(rp)), old_meta(meta) {}
