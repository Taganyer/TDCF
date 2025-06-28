//
// Created by taganyer on 25-5-23.
//

#include <tdcf/base/Errors.hpp>
#include <tdcf/node/Node.hpp>

using namespace tdcf;

Node::Node(IdentityPtr ip, CommunicatorPtr cp, ProcessorPtr pp) :
    _info(std::move(ip), std::move(cp), std::move(pp)) {}

Node::Node(IdentityPtr ip, CommunicatorPtr cp, ProcessorPtr pp, IdentityPtr root_id) :
    _info(std::move(ip), std::move(cp), std::move(pp), std::move(root_id)) {}

void Node::start(unsigned) {
    TDCF_CHECK_EXPR(!_node_agent_started)
    _info.connect(_info.root_id());

    StatusFlag flag;
    do {
        flag = _info.get_communicator_events();
    } while (flag == StatusFlag::CommunicatorGetEventsFurtherWaiting);
    TDCF_CHECK_SUCCESS(flag)
    assert(!_info.message_queue.empty());

    auto [type, id, meta, agent] = _info.message_queue.front();
    _info.message_queue.pop();
    assert(type == CommunicatorEvent::ReceivedMessage && id == _info.root_id());
    assert(meta.operation_type == OperationType::AgentCreate);

    _agent = std::dynamic_pointer_cast<NodeAgent>(agent);
    TDCF_CHECK_EXPR(_agent);
    _info.agent_factory = nullptr;

    flag = _agent->init(meta, _info);
    TDCF_CHECK_SUCCESS(flag)
    _node_agent_started = true;
}

void Node::end_agent() {
    assert(_info.progress_events.size() - _cluster_events == 0);
    _node_agent_started = false;
    _agent = nullptr;
}

StatusFlag Node::handle_message(CommunicatorEvent& event) {
    auto& [type, id, meta, data] = event;
    StatusFlag flag;
    switch (type) {
        case CommunicatorEvent::ReceivedMessage:
            flag = _agent->handle_received_message(id, meta, data, _info);
            break;
        case CommunicatorEvent::MessageSendable:
            flag = _info.send_delay_message(id);
            break;
        default:
            TDCF_RAISE_ERROR("Recieved wrong event type");
    }
    return flag;
}

StatusFlag Node::handle_progress_task(NodeInformation::ProgressTask& task) {
    auto& [iter, meta, result] = task;
    auto& [_, event_progress] = *iter;
    StatusFlag flag;
    if (meta.operation_type == OperationType::Error) {
        event_progress->handle_error(_info);
        flag = StatusFlag::EventEnd;
    } else {
        flag = event_progress->handle_event(meta, result, _info);
    }
    if (flag == StatusFlag::EventEnd) {
        if (meta.progress_type == ProgressType::Root) {
            assert(_cluster_events);
            --_cluster_events;
        }
        _info.progress_events.erase(iter);
        flag = StatusFlag::Success;
    }
    return flag;
}

StatusFlag Node::active_communicator_events() {
    return _info.get_communicator_events();
}

StatusFlag Node::active_processor_events() {
    return _info.get_progress_tasks();
}

StatusFlag Node::handle_communicator_events() {
    auto size = _info.message_queue.size();
    StatusFlag flag = StatusFlag::Success;
    while (size && flag == StatusFlag::Success) {
        assert(size <= _info.message_queue.size());
        --size;
        flag = handle_message(_info.message_queue.front());
        if (flag == StatusFlag::EventEnd) {
            if (_info.message_queue.front().meta.progress_type == ProgressType::Root) {
                assert(_cluster_events);
                --_cluster_events;
            }
            _info.progress_events.erase(_info.message_queue.front().meta);
            flag = StatusFlag::Success;
        } else if (unlikely(flag == StatusFlag::ClusterOffline)) {
            end_agent();
            flag = StatusFlag::Success;
        }
        /// TODO: 这里出错丢弃事件
        _info.message_queue.pop();
    }
    return flag;
}

StatusFlag Node::handle_processor_events() {
    auto size = _info.processed_queue.size();
    StatusFlag flag = StatusFlag::Success;
    while (size && flag == StatusFlag::Success) {
        assert(size <= _info.processed_queue.size());
        --size;
        flag = handle_progress_task(_info.processed_queue.front());
        /// TODO: 这里出错丢弃事件
        _info.message_queue.pop();
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
