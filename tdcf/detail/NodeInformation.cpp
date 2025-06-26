//
// Created by taganyer on 25-6-14.
//

#include <tdcf/base/Errors.hpp>
#include <tdcf/detail/NodeInformation.hpp>
#include <utility>

using namespace tdcf;

void NodeInformation::connect(const IdentityPtr& id) const {
    StatusFlag flag = _communicator->connect(id);
    TDCF_CHECK_SUCCESS(flag)
}

void NodeInformation::accept(const IdentityPtr& id) const {
    StatusFlag flag = _communicator->accept(id);
    TDCF_CHECK_SUCCESS(flag)
}

void NodeInformation::disconnect(const IdentityPtr& id) const {
    StatusFlag flag = _communicator->disconnect(id);
    TDCF_CHECK_SUCCESS(flag)
}

StatusFlag NodeInformation::get_communicator_events() {
    return _communicator->get_events(message_queue);
}

StatusFlag NodeInformation::send_message(const IdentityPtr& id, const MetaData& meta, SerializablePtr message) {
    if (_message_delay[id].empty()) {
        StatusFlag flag = _communicator->send_message(id, Message(meta), message);
        if (flag != StatusFlag::FurtherWaiting) return flag;
    }
    _message_delay[id].emplace(meta, std::move(message));
    return StatusFlag::Success;
}

void NodeInformation::send_delay_message(const IdentityPtr& id) {
    auto& q = _message_delay[id];
    while (!q.empty()) {
        StatusFlag flag = _communicator->send_message(id, Message(q.front().first), q.front().second);
        if (flag == StatusFlag::FurtherWaiting) break;
        TDCF_CHECK_SUCCESS(flag)
        q.pop();
    }
}

bool NodeInformation::delayed_message(const IdentityPtr& id) {
    auto& q = _message_delay[id];
    return !q.empty();
}

Version NodeInformation::get_data_version() {
    Version version = ++_data_version;
    while (_process_delay.find(version) != _process_delay.end()) ++version;
    return version;
}

StatusFlag NodeInformation::get_progress_tasks() {
    StatusFlag flag = _processor->get_events(_data_queue);
    if (flag != StatusFlag::Success) return flag;
    while (!_data_queue.empty()) {
        auto event = std::move(_data_queue.front());
        _data_queue.pop();
        auto iter = _process_delay.find(event.version);
        assert(iter != _process_delay.end());
        processed_queue.emplace(iter->second.first, iter->second.second, std::move(event.result));
    }
    return StatusFlag::Success;
}

void NodeInformation::acquire_data(ProgressEventsMI iter, const MetaData& meta,
                                   const ProcessingRulesPtr& rule_ptr) {

    auto [i, success] = _process_delay.emplace(get_version(), std::pair(iter, meta));
    assert(success);
    _processor->acquire(_data_version, rule_ptr);
}

void NodeInformation::store_data(const ProcessingRulesPtr& rule_ptr, const DataPtr& data_ptr) const {
    _processor->store(rule_ptr, data_ptr);
}

void NodeInformation::reduce_data(ProgressEventsMI iter, const MetaData& meta,
                                  const ProcessingRulesPtr& rule_ptr, const DataSet& target) {
    auto [i, success] = _process_delay.emplace(get_data_version(), std::pair(iter, meta));
    assert(success);
    _processor->reduce(_data_version, rule_ptr, target);
}

void NodeInformation::scatter_data(ProgressEventsMI iter, const MetaData& meta,
                                   const ProcessingRulesPtr& rule_ptr,
                                   unsigned scatter_size, const DataPtr& data_ptr) {
    auto [i, success] = _process_delay.emplace(get_data_version(), std::pair(iter, meta));
    assert(success);
    _processor->scatter(_data_version, rule_ptr, scatter_size, data_ptr);
}
