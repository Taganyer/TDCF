//
// Created by taganyer on 25-5-23.
//

#include <tdcf/base/Errors.hpp>
#include <tdcf/node/Node.hpp>

using namespace tdcf;

Node::Node(IdentityPtr ip, CommunicatorPtr cp, ProcessorPtr pp) :
    _handle(std::move(ip), std::move(cp), std::move(pp)) {}

Node::Node(IdentityPtr ip, CommunicatorPtr cp, ProcessorPtr pp, IdentityPtr root_id) :
    _handle(std::move(ip), std::move(cp), std::move(pp), std::move(root_id)) {}

void Node::start(unsigned) {
    TDCF_CHECK_EXPR(!_node_agent_started)
    _handle.connect(_handle.root_identity());
    _handle.start_communicator_handle();

    MetaData meta = get_agent();
    _handle.agent_factory = nullptr;
    StatusFlag flag = _agent->init(meta, _handle);
    TDCF_CHECK_SUCCESS(flag)
    _handle.set_root_serial(_handle.get_identity_serial(_handle.root_identity()));
    _node_agent_started = true;
}

MetaData Node::get_agent() {
    StatusFlag flag;
    do {
        flag = _handle.get_communicator_events();
    } while (flag == StatusFlag::CommunicatorGetEventsFurtherWaiting);
    TDCF_CHECK_SUCCESS(flag)
    assert(_handle.communicator_events_size());

    auto [from_id, type, meta, agent] = _handle.get_message();
    assert(from_id == _handle.root_serial());
    assert(meta.operation_type == OperationType::AgentCreate);
    _agent = std::dynamic_pointer_cast<NodeAgent>(agent);
    TDCF_CHECK_EXPR(_agent);
    return meta;
}

void Node::end_agent() {
    assert(_handle.progress_events.size() - _cluster_events == 0);
    _node_agent_started = false;
    _agent = nullptr;
}

StatusFlag Node::handle_message(Handle::MessageEvent& event) {
    auto& [from_id, type, meta, data] = event;
    StatusFlag flag;
    switch (type) {
        case CommunicatorEvent::ReceivedMessage:
            flag = _agent->handle_received_message(from_id, meta, data, _handle);
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
    StatusFlag flag;
    if (meta.operation_type == OperationType::Error) {
        event_progress->handle_error(_handle);
        flag = StatusFlag::EventEnd;
    } else {
        flag = event_progress->handle_event(meta, result, _handle);
    }
    if (flag == StatusFlag::EventEnd) {
        if (meta.progress_type == ProgressType::Root) {
            assert(_cluster_events);
            --_cluster_events;
        }
        _handle.progress_events.erase(iter);
        flag = StatusFlag::Success;
    }
    return flag;
}

StatusFlag Node::active_communicator_events() {
    return _handle.get_communicator_events();
}

StatusFlag Node::active_processor_events() {
    return _handle.get_processor_events();
}

StatusFlag Node::handle_communicator_events() {
    auto size = _handle.communicator_events_size();
    StatusFlag flag = StatusFlag::Success;
    while (size && flag == StatusFlag::Success) {
        assert(size <= _handle.communicator_events_size());
        --size;
        auto message = _handle.get_message();
        flag = handle_message(message);
        if (flag == StatusFlag::EventEnd) {
            if (message.meta.progress_type == ProgressType::Root) {
                assert(_cluster_events);
                --_cluster_events;
            }
            _handle.progress_events.erase(message.meta);
            flag = StatusFlag::Success;
        } else if (unlikely(flag == StatusFlag::ClusterOffline)) {
            end_agent();
            flag = StatusFlag::Success;
        }
    }
    return flag;
}

StatusFlag Node::handle_processor_events() {
    auto size = _handle.processor_event_size();
    StatusFlag flag = StatusFlag::Success;
    while (size && flag == StatusFlag::Success) {
        assert(size <= _handle.processor_event_size());
        --size;
        Handle::ProgressTask task;
        if (!_handle.get_task(task)) break;
        flag = handle_progress_task(task);
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
