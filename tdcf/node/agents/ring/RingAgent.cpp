//
// Created by taganyer on 25-7-6.
//

#include <tdcf/base/Errors.hpp>
#include <tdcf/handle/Handle.hpp>
#include <tdcf/node/agents/ring/RingAgent.hpp>

using namespace tdcf;

void RingAgent::init(const IdentityPtr& from_id, const MetaData& meta, Handle& handle) {
    assert(meta.stage == Ring::start);
    handle.create_agent_data<RingAgentData>(nullptr, from_id, meta.serial);

    uint32_t serial = meta.serial;
    while (serial) {
        Handle::MessageEvent event;
        handle.waiting_for_message(event);
        assert(event.type == CommunicatorEvent::ReceivedMessage);
        connect_handle(event, handle);
        --serial;
    }

    waiting_respond(handle);
}

void RingAgent::agent_start(Handle& handle) {
    auto& [send, receive, serial] = handle.agent_data<RingAgentData>();
    MetaData meta;
    meta.stage = Ring::respond;
    StatusFlag flag = handle.send_message(send, meta, nullptr);
    TDCF_CHECK_SUCCESS(flag)
}

StatusFlag RingAgent::handle_disconnect(const IdentityPtr& id, Handle& handle) {
    auto& [send, receive, serial] = handle.agent_data<RingAgentData>();
    assert(equal_to(receive, id));
    handle.disconnect(receive);
    if (!equal_to(send, receive)) {
        handle.disconnect(send);
    }
    return StatusFlag::ClusterOffline;
}

void RingAgent::connect_handle(Handle::MessageEvent& event, Handle& handle) {
    auto& [send, receive, serial] = handle.agent_data<RingAgentData>();
    auto& [type, from, meta, variant] = event;
    assert(meta.operation_type == OperationType::Init);
    assert(equal_to(receive, from));
    auto& ptr = std::get<SerializablePtr>(variant);
    if (!send) {
        send = std::dynamic_pointer_cast<Identity>(ptr);
        if (!equal_to(send, receive)) {
            handle.connect(send);
        }
        if (meta.serial > 1) {
            meta.operation_type = OperationType::AgentCreate;
            meta.data1[0] = ClusterType::ring;
            meta.stage = Ring::start;
            --meta.serial;
            handle.send_message(send, meta, nullptr);
            meta.operation_type = OperationType::Init;
        }
    } else {
        StatusFlag flag = handle.send_message(send, meta, std::move(ptr));
        TDCF_CHECK_SUCCESS(flag)
    }
}

void RingAgent::waiting_respond(Handle& handle) {
    auto& [send, receive, serial] = handle.agent_data<RingAgentData>();
    Handle::MessageEvent event;
    handle.waiting_for_message(event);
    assert(event.type == CommunicatorEvent::ReceivedMessage);
    assert(equal_to(event.id, receive));
    assert(event.meta.stage == Ring::respond);
}

StatusFlag RingAgent::create_progress(uint32_t version, const MetaData& meta,
                                      ProcessingRulesPtr& rule, Handle& handle) {
    switch (meta.operation_type) {
        case OperationType::Broadcast:
            return Broadcast::create(version, meta, rule, handle);
        case OperationType::Scatter:
            return Scatter::create(version, meta, rule, handle);
        case OperationType::Reduce:
            return Reduce::create(version, meta, rule, handle);
        case OperationType::AllReduce:
            return AllReduce::create(version, meta, rule, handle);
        case OperationType::ReduceScatter:
            return ReduceScatter::create(version, meta, rule, handle);
        default:
            TDCF_RAISE_ERROR(error OperationType)
    }
}
