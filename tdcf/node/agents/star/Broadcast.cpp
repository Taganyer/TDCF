//
// Created by taganyer on 25-6-16.
//

#include <tdcf/base/Errors.hpp>
#include <tdcf/handle/Handle.hpp>
#include <tdcf/node/agents/star/StarAgent.hpp>

using namespace tdcf;

StarAgent::Broadcast::Broadcast(uint32_t version, ProcessingRulesPtr rp) :
    EventProgress(OperationType::Broadcast, ProgressType::NodeRoot, version, std::move(rp)) {}

StatusFlag StarAgent::Broadcast::create(const MetaData& meta,
                                        ProcessingRulesPtr rp, Handle& handle) {
    assert(meta.operation_type == OperationType::Broadcast);
    assert(meta.stage == NodeAgentBroadcast::get_rule);

    uint32_t version = handle.create_conversation_version();
    auto iter = handle.create_progress(
        std::make_unique<Broadcast>(version, std::move(rp)));

    if (!handle.agent_factory) return StatusFlag::Success;

    auto& self = static_cast<Broadcast&>(*iter->second);

    StatusFlag flag = handle.agent_factory->broadcast(self.rule, iter, handle, &self._agent);
    if (flag != StatusFlag::Success || !self._agent) {
        handle.destroy_progress(iter);
        return flag;
    }

    return StatusFlag::Success;
}

StatusFlag StarAgent::Broadcast::handle_event(const MetaData& meta,
                                              Variant& data, Handle& handle) {
    assert(meta.operation_type == OperationType::Broadcast);
    if (!_agent) {
        assert(meta.stage == NodeAgentBroadcast::get_data);
        handle.store_data(rule, std::get<DataPtr>(data));
        return close(handle);
    }
    if (meta.stage == NodeAgentBroadcast::get_data) {
        return agent_store(data, handle);
    }
    /// 此时 _agent 指向的对象已销毁。
    if (meta.stage == NodeAgentBroadcast::finish_ack) {
        return close(handle);
    }
    TDCF_RAISE_ERROR(meta.stage error type)
}

StatusFlag StarAgent::Broadcast::agent_store(Variant& data, Handle& handle) const {
    MetaData meta = create_meta();
    meta.stage = NodeAgentBroadcast::send_data;
    return _agent->proxy_event(meta, data, handle);
}

StatusFlag StarAgent::Broadcast::close(Handle& handle) const {
    MetaData meta = create_meta();
    meta.stage = NodeAgentBroadcast::finish;
    assert(handle.root_identity());
    StatusFlag flag = handle.send_progress_message(version, handle.root_identity(), meta, nullptr);
    if (flag != StatusFlag::Success) return flag;
    return StatusFlag::EventEnd;
}
