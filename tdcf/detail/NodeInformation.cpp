//
// Created by taganyer on 25-6-14.
//

#include <tdcf/base/Errors.hpp>
#include <tdcf/detail/NodeInformation.hpp>
#include <utility>

using namespace tdcf;

void NodeInformation::connect(const IdentityPtr& id) const {
    bool success = _communicator->connect(id);
    TDCF_CHECK_EXPR(success)
}

void NodeInformation::accept(const IdentityPtr& id) const {
    bool success = _communicator->accept(id);
    TDCF_CHECK_EXPR(success)
}

void NodeInformation::disconnect(const IdentityPtr& id) const {
    bool success = _communicator->disconnect(id);
    TDCF_CHECK_EXPR(success)
}

StatusFlag NodeInformation::get_communicator_events() {
    OperationFlag flag = _communicator->get_events(message_queue);
    switch (flag) {
        case OperationFlag::Success: return StatusFlag::Success;
        case OperationFlag::FurtherWaiting: return StatusFlag::CommunicatorGetEventsFurtherWaiting;
        case OperationFlag::Error: return StatusFlag::CommunicatorGetEventsError;
    }
    TDCF_RAISE_ERROR(unknown type)
}

StatusFlag NodeInformation::send_message(const IdentityPtr& id, const MetaData& meta, SerializablePtr message) {
    if (_message_delay[id].empty()) {
        OperationFlag flag = _communicator->send_message(id, Message(meta), message);
        if (flag == OperationFlag::Success) return StatusFlag::Success;
        if (flag == OperationFlag::Error) return StatusFlag::CommunicatorSendMessageError;
    }
    _message_delay[id].emplace(meta, std::move(message));
    return StatusFlag::Success;
}

StatusFlag NodeInformation::send_delay_message(const IdentityPtr& id) {
    auto& q = _message_delay[id];
    while (!q.empty()) {
        OperationFlag flag = _communicator->send_message(id, Message(q.front().first), q.front().second);
        if (flag == OperationFlag::FurtherWaiting) break;
        if (unlikely(flag == OperationFlag::Error)) return StatusFlag::CommunicatorSendMessageError;
        q.pop();
    }
    return StatusFlag::Success;
}

bool NodeInformation::delayed_message(const IdentityPtr& id) {
    auto& q = _message_delay[id];
    return !q.empty();
}

ProcessorEventMark NodeInformation::get_mark(ProgressEventsMI iter) {
    return { iter->first.version(), ++_data_version.version };
}

StatusFlag NodeInformation::get_progress_tasks() {
    OperationFlag flag = _processor->get_events(_data_queue);
    if (flag != OperationFlag::Success) {
        return flag == OperationFlag::Error ? StatusFlag::ProcessorGetEventsError :
                   StatusFlag::ProcessorGetEventsFurtherWaiting;
    }
    while (!_data_queue.empty()) {
        auto [type, mark, result] = std::move(_data_queue.front());
        _data_queue.pop();
        auto iter = _process_delay.find(mark);
        if (iter == _process_delay.end()) continue;
        MetaData meta = iter->second.second;
        if (type == ProcessorEvent::Error)
            meta.operation_type = OperationType::Error;
        processed_queue.emplace(iter->second.first, meta, std::move(result));
        _process_delay.erase(iter);
    }
    return StatusFlag::Success;
}

void NodeInformation::acquire_data(ProgressEventsMI iter, const MetaData& meta,
                                   const ProcessingRulesPtr& rule_ptr) {
    ProcessorEventMark mark = get_mark(iter);
    auto [i, success] = _process_delay.emplace(mark, std::pair(iter, meta));
    assert(success);
    _processor->acquire(mark, rule_ptr);
}

void NodeInformation::store_data(const ProcessingRulesPtr& rule_ptr, const DataPtr& data_ptr) const {
    _processor->store(rule_ptr, data_ptr);
}

void NodeInformation::reduce_data(ProgressEventsMI iter, const MetaData& meta,
                                  const ProcessingRulesPtr& rule_ptr, const DataSet& target) {
    ProcessorEventMark mark = get_mark(iter);
    auto [i, success] = _process_delay.emplace(mark, std::pair(iter, meta));
    assert(success);
    _processor->reduce(mark, rule_ptr, target);
}

void NodeInformation::scatter_data(ProgressEventsMI iter, const MetaData& meta,
                                   const ProcessingRulesPtr& rule_ptr,
                                   unsigned scatter_size, const DataPtr& data_ptr) {
    ProcessorEventMark mark = get_mark(iter);
    auto [i, success] = _process_delay.emplace(mark, std::pair(iter, meta));
    assert(success);
    _processor->scatter(mark, rule_ptr, scatter_size, data_ptr);
}
