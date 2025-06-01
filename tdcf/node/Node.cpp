//
// Created by taganyer on 25-5-23.
//

#include <tdcf/base/Errors.hpp>
#include <tdcf/node/Node.hpp>

using namespace tdcf;

StatusFlag NodeInformation::send_message(const IdentityPtr& id, SerializablePtr message) {
    if (!send_delay[id].empty()) {
        CommunicatorEvent event { CommunicatorType::SendMessage, id, message };
        StatusFlag flag = communicator->add_event(std::move(event));
        if (flag != StatusFlag::FurtherWaiting) return flag;
    }
    send_delay[id].emplace(std::move(message));
    return StatusFlag::Success;
}

StatusFlag NodeInformation::send_delay_message(const IdentityPtr& id) {
    auto& q = send_delay[id];
    while (!q.empty()) {
        CommunicatorEvent event { CommunicatorType::MessageSendable, id, q.front() };
        StatusFlag flag = communicator->add_event(std::move(event));
        if (flag != StatusFlag::Success) return flag;
        q.pop();
    }
    return StatusFlag::Success;
}

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
        flag = _info.communicator->get_event(_data.message_queue);
    TDCF_CHECK_SUCCESS(flag)
    assert(!_data.message_queue.empty());

    auto& [type, id, message] = _data.message_queue.front();
    assert(type == CommunicatorType::SendMessage && id == _info.root_id);
    _agent = std::dynamic_pointer_cast<NodeAgent>(message);
    assert(_agent);
    flag = _agent->init(_info, _data);
    TDCF_CHECK_SUCCESS(flag)
}

StatusFlag Node::agent_analysis_message(CommunicatorEvent& event) {
    return _agent->analysis_message(_data, event);
}

StatusFlag Node::agent_handle_message(CommunicatorEvent& event) {
    auto& [type, id, message] = event;
    StatusFlag flag = StatusFlag::Success;
    switch (type) {
        case CommunicatorType::ReceivedMessage:
            flag = _agent->handle_received_message(_info, _data, id, message);
            break;
        case CommunicatorType::MessageSendable:
            flag = _info.send_delay_message(id);
            break;
        case CommunicatorType::ConnectRequest:
            flag = _agent->handle_connect_request(_info, _data, id);
            break;
        case CommunicatorType::DisconnectRequest:
            flag = _agent->handle_disconnect_request(_info, _data, id);
            break;
        default:
            TDCF_RAISE_ERROR("Recieved wrong event type");
    }
    return flag;
}

StatusFlag Node::active_events() {
    return _info.communicator->get_event(_data.message_queue);
}

StatusFlag Node::analysis_messages() {
    while (!_data.message_queue.empty()) {
        StatusFlag flag = agent_analysis_message(_data.message_queue.front());
        if (flag != StatusFlag::Success) return flag;
        _data.message_queue.pop();
    }
    return StatusFlag::Success;
}

StatusFlag Node::handle_messages() {
    while (!_data.message_queue.empty()) {
        StatusFlag flag = agent_handle_message(_data.message_queue.front());
        if (flag != StatusFlag::Success) return flag;
        _data.message_queue.pop();
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
