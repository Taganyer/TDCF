//
// Created by taganyer on 25-6-29.
//

#include <algorithm>
#include <tdcf/base/Errors.hpp>
#include <tdcf/detail/Message.hpp>
#include <tdcf/handle/CommunicatorHandle.hpp>

using namespace tdcf;

void CommunicatorHandle::connect(const IdentityPtr& identity) {
    bool success = _communicator->connect(identity);
    TDCF_CHECK_EXPR(success)
    _id_list.push_back(identity);
}

void CommunicatorHandle::accept(const IdentityPtr& identity) {
    bool success = _communicator->accept(identity);
    TDCF_CHECK_EXPR(success)
    _id_list.push_back(identity);
}

void CommunicatorHandle::disconnect(uint32_t id) {
    bool success = _communicator->disconnect(_id_list[id]);
    TDCF_CHECK_EXPR(success)
    _id_list.erase(_id_list.begin() + id);
}

void CommunicatorHandle::start_communicator_handle() {
    std::sort(_id_list.begin(), _id_list.end());
}

uint32_t CommunicatorHandle::create_conversation_version() {
    uint32_t ret = _version++.version;
    return ret;
}

void CommunicatorHandle::close_conversation(uint32_t version) {
    std::pair key(version, 0);
    auto iter = _send.lower_bound(key);
    assert(iter != _send.end());
    while (iter != _send.end() && iter->first.first == version) {
        auto [k, v] = *iter;
        std::pair rk(v, k.second);
        auto size = _receive.erase(rk);
        assert(size == 1);
        iter = _send.erase(iter);
    }
}

StatusFlag CommunicatorHandle::get_communicator_events() {
    OperationFlag flag = _communicator->get_events(_receive_queue);
    switch (flag) {
        case OperationFlag::Success: return StatusFlag::Success;
        case OperationFlag::FurtherWaiting: return StatusFlag::CommunicatorGetEventsFurtherWaiting;
        case OperationFlag::Error: return StatusFlag::CommunicatorGetEventsError;
    }
    TDCF_RAISE_ERROR(unknown type)
}

CommunicatorHandle::MessageEvent CommunicatorHandle::get_message() {
    auto [type, id, meta, data] = std::move(_receive_queue.front());
    _receive_queue.pop();
    if ((int) type > (int) CommunicatorEvent::ReceivedMessage) {
        MessageEvent event { 0, type, meta, std::move(id) };
        return event;
    }
    uint32_t from = get_identity_serial(id);
    uint32_t target_version = 0;
    if (type == CommunicatorEvent::ReceivedMessage) {
        target_version = receive_transition(from, meta);
    }
    meta.version = target_version;
    MessageEvent event { from, type, meta, std::move(data) };
    return event;
}

StatusFlag CommunicatorHandle::send_message(uint32_t version, uint32_t target,
                                            MetaData meta, SerializablePtr message) {
    if ((int) meta.operation_type > (int) OperationType::Close) {
        send_transition(version, target, meta);
    }
    if (_delay_queue[target].empty()) {
        OperationFlag flag = _communicator->send_message(_id_list[target], Message(meta), message);
        if (flag == OperationFlag::Success) return StatusFlag::Success;
        if (flag == OperationFlag::Error) return StatusFlag::CommunicatorSendMessageError;
    }
    _delay_queue[target].emplace(meta, std::move(message));
    return StatusFlag::Success;
}

StatusFlag CommunicatorHandle::send_delay_message(uint32_t target) {
    auto& q = _delay_queue[target];
    while (!q.empty()) {
        OperationFlag flag = _communicator->send_message(_id_list[target], Message(q.front().first), q.front().second);
        if (flag == OperationFlag::FurtherWaiting) break;
        if (unlikely(flag == OperationFlag::Error)) return StatusFlag::CommunicatorSendMessageError;
        q.pop();
    }
    return StatusFlag::Success;
}

bool CommunicatorHandle::delayed_message(uint32_t target) {
    auto& q = _delay_queue[target];
    return !q.empty();
}

uint32_t CommunicatorHandle::id_list_size() const {
    return _id_list.size();
}

uint32_t CommunicatorHandle::communicator_events_size() const {
    return _receive_queue.size();
}

uint32_t CommunicatorHandle::get_identity_serial(const IdentityPtr& identity) const {
    auto iter = std::lower_bound(_id_list.begin(), _id_list.end(), identity);
    TDCF_CHECK_EXPR(iter != _id_list.end());
    assert(*iter == identity);
    return iter - _id_list.begin();
}

void CommunicatorHandle::send_transition(uint32_t version, uint32_t to, MetaData& meta) {
    std::pair key(version, to);
    auto iter = _send.find(key);
    if (iter == _send.end()) {
        auto [i, success] = _send.emplace(key, version);
        assert(success);
        iter = i;
        auto [ii, ss] = _receive.emplace(key, version);
        assert(ss);
    }
    meta.version = iter->second;
}

uint32_t CommunicatorHandle::receive_transition(uint32_t from, const MetaData& meta) {
    std::pair key(meta.version, from);
    auto iter = _receive.find(key);
    uint32_t target_version;
    if (iter == _receive.end()) {
        target_version = create_conversation_version();
        auto [i, success] = _receive.emplace(key, target_version);
        assert(success);
        key.first = target_version;
        auto [ii, ss] = _send.emplace(key, meta.version);
        assert(ss);
    } else {
        target_version = iter->second;
    }
    return target_version;
}
