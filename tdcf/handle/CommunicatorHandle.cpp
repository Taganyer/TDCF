//
// Created by taganyer on 25-6-29.
//

#include <algorithm>
#include <tdcf/base/Errors.hpp>
#include <tdcf/detail/Message.hpp>
#include <tdcf/handle/CommunicatorHandle.hpp>

using namespace tdcf;

void CommunicatorHandle::connect(const IdentityPtr& identity) const {
    bool success = _communicator->connect(identity);
    TDCF_CHECK_EXPR(success)
}

void CommunicatorHandle::accept(const IdentityPtr& identity) const {
    bool success = _communicator->accept(identity);
    TDCF_CHECK_EXPR(success)
}

void CommunicatorHandle::disconnect(const IdentityPtr& id) const {
    bool success = _communicator->disconnect(id);
    TDCF_CHECK_EXPR(success)
}

uint32_t CommunicatorHandle::create_conversation_version() {
    uint32_t ret = _version.version;
    ++_version;
    return ret;
}

void CommunicatorHandle::close_conversation(uint32_t version) {
    Key key(version, nullptr);
    auto iter = _send.lower_bound(key);
    assert(iter != _send.end());
    while (iter != _send.end() && iter->first.first == version) {
        auto [k, v] = *iter;
        Key rk(v, k.second);
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

bool CommunicatorHandle::get_message(CommunicatorEvent& message) {
    while (!_receive_queue.empty()) {
        auto [type, id, meta, data] = std::move(_receive_queue.front());
        _receive_queue.pop();
        if (meta.link_mark == LinkMark::Create) {
            meta.version = create_receive_link(meta.version, id);
        } else if (meta.link_mark == LinkMark::Info) {
            if (!receive_transition(id, meta)) continue;
        }
        message = { type, std::move(id), meta, std::move(data) };
        return true;
    }
    return false;
}

StatusFlag CommunicatorHandle::send_message(const IdentityPtr& target,
                                            MetaData meta, SerializablePtr message) {
    meta.link_mark = LinkMark::Null;
    StatusFlag flag = send(target, meta, std::move(message));
    return flag;
}

StatusFlag CommunicatorHandle::start_progress_message(uint32_t version, const IdentityPtr& target,
                                                      MetaData meta, SerializablePtr message) {
    create_send_link(version, target);
    meta.version = version;
    meta.link_mark = LinkMark::Create;
    StatusFlag flag = send(target, meta, std::move(message));
    return flag;
}

StatusFlag CommunicatorHandle::send_progress_message(uint32_t version, const IdentityPtr& target,
                                                     MetaData meta, SerializablePtr message) {
    send_transition(version, target, meta);
    meta.link_mark = LinkMark::Info;
    StatusFlag flag = send(target, meta, std::move(message));
    return flag;
}

StatusFlag CommunicatorHandle::send_delay_message(const IdentityPtr& target) {
    auto& q = _delay_queue[target];
    while (!q.empty()) {
        OperationFlag flag = _communicator->send_message(target, Message(q.front().first), q.front().second);
        if (flag == OperationFlag::FurtherWaiting) break;
        if (unlikely(flag == OperationFlag::Error)) return StatusFlag::CommunicatorSendMessageError;
        q.pop();
    }
    return StatusFlag::Success;
}

bool CommunicatorHandle::delayed_message(const IdentityPtr& target) {
    auto& q = _delay_queue[target];
    return !q.empty();
}

StatusFlag CommunicatorHandle::send(const IdentityPtr& target,
                                    MetaData meta, SerializablePtr message) {
    if (_delay_queue[target].empty()) {
        OperationFlag flag = _communicator->send_message(target, Message(meta), message);
        if (flag == OperationFlag::Success) return StatusFlag::Success;
        if (flag == OperationFlag::Error) return StatusFlag::CommunicatorSendMessageError;
    }
    _delay_queue[target].emplace(meta, std::move(message));
    return StatusFlag::Success;
}

void CommunicatorHandle::create_send_link(uint32_t version, const IdentityPtr& to) {
    Key key(version, to);
    assert(_send.find(key) == _send.end());
    auto [i, success] = _send.emplace(key, version);
    assert(success);
    auto [ii, ss] = _receive.emplace(key, version);
    assert(ss);
}

uint32_t CommunicatorHandle::create_receive_link(uint32_t from_version, const IdentityPtr& from) {
    uint32_t version = create_conversation_version();
    Key key(from_version, from);
    auto [i, success] = _receive.emplace(key, version);
    assert(success);
    key.first = version;
    auto [ii, ss] = _send.emplace(key, from_version);
    assert(ss);
    return version;
}

void CommunicatorHandle::send_transition(uint32_t version,
                                         const IdentityPtr& to, MetaData& meta) const {
    Key key(version, to);
    auto iter = _send.find(key);
    TDCF_CHECK_EXPR(iter != _send.end())
    meta.version = iter->second;
}

bool CommunicatorHandle::receive_transition(const IdentityPtr& from, MetaData& meta) const {
    Key key(meta.version, from);
    auto iter = _receive.find(key);
    if (iter == _receive.end()) return false;
    meta.version = iter->second;
    return true;
}
