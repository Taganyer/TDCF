//
// Created by taganyer on 25-5-23.
//

#include <tdcf/base/Errors.hpp>
#include <tdcf/node/Node.hpp>

using namespace tdcf;

Node::Node(IdentityPtr ip, CommunicatorPtr cp, ProcessorPtr pp,
           IdentityPtr root_id) :
    _info(std::move(ip), std::move(cp), std::move(pp), std::move(root_id)) {
    assert(_info.check());
    join_in_cluster();
}

void Node::join_in_cluster() {
    TDCF_CHECK_SUCCESS(_info.communicator->connect(_info.root_id))
    StatusFlag flag = StatusFlag::FurtherWaiting;
    while (flag == StatusFlag::FurtherWaiting)
        flag = _info.communicator->get_events(_info.message_queue);
    TDCF_CHECK_SUCCESS(flag)
    assert(!_info.message_queue.empty());

    auto& [type, id, meta, message] = _info.message_queue.front();
    assert(type == CommunicatorEvent::ReceivedMessage && id == _info.root_id);
    _agent = std::dynamic_pointer_cast<NodeAgent>(message);
    assert(_agent);
    flag = _agent->init(_info);
    TDCF_CHECK_SUCCESS(flag)
}

StatusFlag Node::agent_analysis_message(CommunicatorEvent& event) {
    return _agent->analysis_message(_info, event);
}

StatusFlag Node::agent_handle_message(CommunicatorEvent& event) {
    auto& [type, id, meta, data] = event;
    StatusFlag flag = StatusFlag::Success;
    switch (type) {
        case CommunicatorEvent::ReceivedMessage:
            flag = _agent->handle_received_message(_info, id, meta, data);
            break;
        case CommunicatorEvent::MessageSendable:
            flag = _info.send_delay_message(id);
            break;
        case CommunicatorEvent::ConnectRequest:
            flag = _agent->handle_connect_request(_info, id);
            break;
        case CommunicatorEvent::DisconnectRequest:
            flag = _agent->handle_disconnect_request(_info, id);
            break;
        default:
            TDCF_RAISE_ERROR("Recieved wrong event type");
    }
    return flag;
}

StatusFlag Node::active_events() {
    StatusFlag flag = _info.get_communicator_event();
    if (flag != StatusFlag::Success) return flag;
    return _info.get_processor_data();
}

StatusFlag Node::analysis_messages() {
    while (!_info.message_queue.empty()) {
        StatusFlag flag = agent_analysis_message(_info.message_queue.front());
        if (flag != StatusFlag::Success) return flag;
        _info.message_queue.pop();
    }
    return StatusFlag::Success;
}

StatusFlag Node::handle_messages() {
    while (!_info.message_queue.empty()) {
        StatusFlag flag = agent_handle_message(_info.message_queue.front());
        if (flag != StatusFlag::Success) return flag;
        _info.message_queue.pop();
    }
    return StatusFlag::Success;
}

StatusFlag Node::handle_a_loop() {
    StatusFlag flag = active_events();
    if (flag != StatusFlag::Success) return flag;
    flag = analysis_messages();
    if (flag != StatusFlag::Success) return flag;
    return handle_messages();
}
