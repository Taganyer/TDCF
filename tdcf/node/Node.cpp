//
// Created by taganyer on 25-5-23.
//

#include <tdcf/base/Errors.hpp>
#include <tdcf/node/Node.hpp>
#include <tdcf/cluster/Cluster.hpp>

using namespace tdcf;

Node::Node(IdentityPtr ip, CommunicatorPtr cp, ProcessorPtr pp) :
    _handle(std::move(ip), std::move(cp), std::move(pp)) {}

Node::~Node()  { assert(!_node_agent_started); }

void Node::start_node() {
    TDCF_CHECK_EXPR(!dynamic_cast<Cluster*>(this) || _cluster_staring);
    TDCF_CHECK_EXPR(!_node_agent_started)
    auto id = _handle.accept();
    MetaData meta = get_agent();
    if (!_cluster_started) _handle.agent_factory = nullptr;
    _agent->init(id, meta, _handle);
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

    auto& [type, from, meta, agent] = message;
    assert(from->equal_to(*from_id));
    assert(meta.operation_type == OperationType::AgentCreate);
    _agent = std::dynamic_pointer_cast<NodeAgent>(std::get<SerializablePtr>(agent));
    TDCF_CHECK_EXPR(_agent);
    return meta;
}

void Node::end_agent() {
    assert(_handle.total_events() - _handle.cluster_events() == 0);
    _node_agent_started = false;
    _agent = nullptr;
    _handle.destroy_agent_data();
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
        case CommunicatorEvent::DisconnectRequest:
            flag = _agent->handle_disconnect(from_id, _handle);
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
    Handle::MessageEvent event;
    while (flag == StatusFlag::Success && _handle.get_message(event)) {
        flag = handle_message(event);
        if (flag == StatusFlag::EventEnd) {
            auto iter = _handle.find_progress(event.meta.version);
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
