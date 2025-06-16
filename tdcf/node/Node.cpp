//
// Created by taganyer on 25-5-23.
//

#include <tdcf/base/Errors.hpp>
#include <tdcf/node/Node.hpp>

using namespace tdcf;

Node::Node(IdentityPtr ip, CommunicatorPtr cp, ProcessorPtr pp) :
    _info(std::move(ip), std::move(cp), std::move(pp)) {}

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

StatusFlag Node::handle_message(CommunicatorEvent& event) {
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

StatusFlag Node::handle_progress_task(NodeInformation::ProgressTask& task) {
    auto& [iter, meta, result] = task;
    auto& [_, event_progress] = *iter;
    StatusFlag flag = event_progress->handle_event(meta, &result, _info);
    if (flag == StatusFlag::EventEnd) {
        _info.progress_events.erase(iter);
        flag = StatusFlag::Success;
    }
    return flag;
}

StatusFlag Node::active_communicator_events() {
    return _info.get_communicator_events();
}

StatusFlag Node::handle_communicator_events() {
    auto size = _info.message_queue.size();
    while (size) {
        --size;
        StatusFlag flag = handle_message(_info.message_queue.front());
        /// TODO: 这里出错不丢弃事件
        if (flag != StatusFlag::Success) return flag;
        _info.message_queue.pop();
    }
    return StatusFlag::Success;
}

StatusFlag Node::active_processor_events() {
    return _info.get_progress_tasks();
}

StatusFlag Node::handle_processor_events() {
    auto size = _info.processed_queue.size();
    while (size) {
        --size;
        StatusFlag flag = handle_progress_task(_info.processed_queue.front());
        /// TODO: 这里出错不丢弃事件
        if (flag != StatusFlag::Success) return flag;
        _info.message_queue.pop();
    }
    return StatusFlag::Success;
}

StatusFlag Node::handle_a_loop() {
    StatusFlag flag = active_communicator_events();
    if (flag != StatusFlag::Success) return flag;
    flag = handle_communicator_events();
    if (flag != StatusFlag::Success) return flag;
    flag = active_processor_events();
    if (flag != StatusFlag::Success) return flag;
    return handle_processor_events();
}
