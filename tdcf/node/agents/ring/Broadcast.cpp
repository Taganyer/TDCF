//
// Created by taganyer on 25-7-8.
//

#include <tdcf/base/Errors.hpp>
#include <tdcf/handle/Handle.hpp>
#include <tdcf/node/agents/ring/RingAgent.hpp>

using namespace tdcf;

RingAgent::Broadcast::Broadcast(uint32_t version, ProcessingRulesPtr rp) :
    EventProgress(OperationType::Broadcast, ProgressType::NodeRoot, version, std::move(rp)) {}

StatusFlag RingAgent::Broadcast::create(uint32_t version, const MetaData& meta,
                                        ProcessingRulesPtr rp, Handle& handle) {
    assert(meta.operation_type == OperationType::Broadcast);
    assert(meta.stage == N_Broadcast::get_rule);

    auto& [send, receive, serial] = handle.agent_data<RingAgentData>();
    assert(meta.serial == serial);

    auto iter = handle.create_progress(std::make_unique<Broadcast>(version, std::move(rp)));

    auto& self = static_cast<Broadcast&>(*iter->second);
    self.serial = serial - 1;

    if (serial != 1) {
        MetaData new_meta = self.create_meta();
        new_meta.stage = C_Broadcast::send_rule;
        StatusFlag flag = handle.send_progress_message(version, send, new_meta, self.rule);
        if (flag != StatusFlag::Success) {
            handle.destroy_progress(iter);
            return flag;
        }
    }

    if (!handle.agent_factory) return StatusFlag::Success;

    StatusFlag flag = handle.agent_factory->broadcast(self.rule, iter, handle, &self._agent);
    if (flag != StatusFlag::Success || !self._agent) {
        handle.destroy_progress(iter);
        return flag;
    }

    return StatusFlag::Success;
}

StatusFlag RingAgent::Broadcast::handle_event(const MetaData& meta, Variant& data, Handle& handle) {
    assert(meta.operation_type == OperationType::Broadcast);
    if (!_agent) {
        assert(meta.stage == N_Broadcast::get_data);
        handle.store_data(rule, std::get<DataPtr>(data));
        send_data(std::get<DataPtr>(data), handle);
        return close(handle);
    }
    if (meta.stage == N_Broadcast::get_data) {
        return agent_store(data, handle);
    }
    if (meta.stage == N_Broadcast::finish_ack) {
        return close(handle);
    }
    TDCF_RAISE_ERROR(meta.stage error type)
}

void RingAgent::Broadcast::send_data(DataPtr& data, Handle& handle) const {
    auto& [send, receive, serial] = handle.agent_data<RingAgentData>();
    if (serial == 1) return;
    MetaData meta = create_meta();
    meta.stage = C_Broadcast::send_data;
    StatusFlag flag = handle.send_progress_message(version, send, meta, data);
    TDCF_CHECK_SUCCESS(flag)
}

StatusFlag RingAgent::Broadcast::agent_store(Variant& data, Handle& handle) const {
    MetaData meta = create_meta();
    meta.stage = N_Broadcast::send_data;
    return _agent->proxy_event(meta, data, handle);
}

StatusFlag RingAgent::Broadcast::close(Handle& handle) const {
    MetaData meta = create_meta();
    meta.stage = N_Broadcast::finish;
    auto& [send, receive, serial] = handle.agent_data<RingAgentData>();
    StatusFlag flag = handle.send_progress_message(version, send, meta, nullptr);
    TDCF_CHECK_SUCCESS(flag)
    return StatusFlag::EventEnd;
}
