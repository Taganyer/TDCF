//
// Created by taganyer on 25-5-23.
//

#include <tdcf/base/Errors.hpp>
#include <tdcf/node/Node.hpp>

using namespace tdcf;

Node::Node(IdentityPtr ip, CommunicatorPtr cp, ProcessorPtr pp, IdentityPtr root_id) :
    _handle(std::move(ip), std::move(cp), std::move(pp), std::move(root_id)) {}

void Node::start(unsigned) {
    TDCF_CHECK_EXPR(!_node_agent_started)
    _handle.connect(_handle.root_identity());

    MetaData meta = get_agent();
    _handle.agent_factory = nullptr;
    StatusFlag flag = _agent->init(meta, _handle);
    TDCF_CHECK_SUCCESS(flag)
    _node_agent_started = true;
}

MetaData Node::get_agent() {
    StatusFlag flag;
    do {
        flag = _handle.get_communicator_events();
    } while (flag == StatusFlag::CommunicatorGetEventsFurtherWaiting);
    TDCF_CHECK_SUCCESS(flag)

    Handle::MessageEvent message;
    bool success = _handle.get_message(message);
    TDCF_CHECK_EXPR(success)

    auto& [type, from_id, meta, agent] = message;
    assert(from_id->equal_to(*_handle.root_identity()));
    assert(meta.operation_type == OperationType::AgentCreate);
    _agent = std::dynamic_pointer_cast<NodeAgent>(std::get<SerializablePtr>(agent));
    TDCF_CHECK_EXPR(_agent);
    return meta;
}

void Node::end_agent() {
    assert(_handle.total_events() - _handle.cluster_events() == 0);
    _node_agent_started = false;
    _agent = nullptr;
}

StatusFlag Node::handle_message(Handle::MessageEvent& event) {
    auto& [type, from_id, meta, variant] = event;
    StatusFlag flag;
    switch (type) {
        case CommunicatorEvent::ReceivedMessage:
            flag = _agent->handle_received_message(from_id, meta, variant, _handle);
            break;
        case CommunicatorEvent::MessageSendable:
            flag = _handle.send_delay_message(from_id);
            break;
        default:
            TDCF_RAISE_ERROR("Recieved wrong event type");
    }
    return flag;
}

StatusFlag Node::handle_progress_task(Handle::ProgressTask& task) {
    auto& [iter, meta, result] = task;
    auto& [_, event_progress] = *iter;
    return event_progress->handle_event(meta, result, _handle);
}

StatusFlag Node::active_communicator_events() {
    return _handle.get_communicator_events();
}

StatusFlag Node::active_processor_events() {
    return _handle.get_processor_events();
}

StatusFlag Node::handle_communicator_events() {
    StatusFlag flag = StatusFlag::Success;
    Handle::MessageEvent message;
    while (flag == StatusFlag::Success && _handle.get_message(message)) {
        flag = handle_message(message);
        if (flag == StatusFlag::EventEnd) {
            auto iter = _handle.find_progress(message.meta.version);
            _handle.destroy_progress(iter);
            flag = StatusFlag::Success;
        } else if (unlikely(flag == StatusFlag::ClusterOffline)) {
            end_agent();
        }
    }
    return flag;
}

StatusFlag Node::handle_processor_events() {
    StatusFlag flag = StatusFlag::Success;
    Handle::ProgressTask task;
    while (flag == StatusFlag::Success && _handle.get_progress_task(task)) {
        flag = handle_progress_task(task);
        if (flag == StatusFlag::EventEnd) {
            _handle.destroy_progress(task.iter);
            flag = StatusFlag::Success;
        }
    }
    return flag;
}

StatusFlag Node::handle_a_loop() {
    if (!_node_agent_started && !_cluster_started)
        return StatusFlag::ClusterOffline;
    StatusFlag flag = active_communicator_events();
    if (flag == StatusFlag::CommunicatorGetEventsError) return flag;
    flag = handle_communicator_events();
    if (flag != StatusFlag::Success) return flag;
    flag = active_processor_events();
    if (flag == StatusFlag::ProcessorGetEventsError) return flag;
    return handle_processor_events();
}
