//
// Created by taganyer on 25-6-29.
//

#include <algorithm>
#include <tdcf/base/Errors.hpp>
#include <tdcf/detail/Message.hpp>
#include <tdcf/handle/CommunicatorHandle.hpp>

using namespace tdcf;

CommunicatorHandle::CommunicatorHandle(CommunicatorPtr ptr) :
    _communicator(std::move(ptr)) {
    TDCF_CHECK_EXPR(_communicator)
}

CommunicatorHandle::~CommunicatorHandle() {
    assert(_receive.empty() && _send.empty());
}

void CommunicatorHandle::connect(const IdentityPtr& identity) const {
    bool success = _communicator->connect(identity);
    TDCF_CHECK_EXPR(success)
}

IdentityPtr CommunicatorHandle::accept() const {
    auto id = _communicator->accept();
    TDCF_CHECK_EXPR(id)
    return id;
}

void CommunicatorHandle::disconnect(const IdentityPtr& id) const {
    TDCF_CHECK_EXPR(!delayed_message(id))
    bool success = _communicator->disconnect(id);
    TDCF_CHECK_EXPR(success)
}

uint32_t CommunicatorHandle::create_conversation_version() {
    ++_version;
    while (_receive.find(_version.version) != _receive.end()
        || _send.find(_version.version) != _send.end()) {
        ++_version;
    }
    return _version.version;
}

void CommunicatorHandle::close_conversation(uint32_t version) {
    auto send_iter = _send.find(version);
    assert(send_iter != _send.end());
    auto receive_iter = _receive.find(send_iter->second);
    assert(receive_iter != _receive.end());
    _send.erase(send_iter);
    _receive.erase(receive_iter);
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

bool CommunicatorHandle::get_message(MessageEvent& message) {
    while (!_receive_queue.empty()) {
        auto [type, id, meta, data] = std::move(_receive_queue.front());
        _receive_queue.pop();
        if (meta.link_mark != LinkMark::Null) {
            receive_transition(meta);
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

StatusFlag CommunicatorHandle::send_progress_message(uint32_t version, const IdentityPtr& target,
                                                     MetaData meta, SerializablePtr message) {
    send_transition(version, meta);
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

bool CommunicatorHandle::delayed_message(const IdentityPtr& target) const {
    if (!target) return false;
    auto iter = _delay_queue.find(target);
    if (iter == _delay_queue.end()) return false;
    auto& q = iter->second;
    return !q.empty();
}

StatusFlag CommunicatorHandle::send(const IdentityPtr& target,
                                    MetaData meta, SerializablePtr message) {
    auto& q = _delay_queue[target];
    if (q.empty()) {
        OperationFlag flag = _communicator->send_message(target, Message(meta), message);
        if (flag == OperationFlag::Success) return StatusFlag::Success;
        if (flag == OperationFlag::Error) return StatusFlag::CommunicatorSendMessageError;
    }
    q.emplace(meta, std::move(message));
    return StatusFlag::Success;
}

void CommunicatorHandle::create_send_link(uint32_t version) {
    auto [i, success] = _send.emplace(version, version);
    assert(success);
    auto [ii, ss] = _receive.emplace(version, version);
    assert(ss);
}

uint32_t CommunicatorHandle::create_receive_link(uint32_t from_version) {
    uint32_t version = create_conversation_version();
    auto [i, success] = _receive.emplace(from_version, version);
    assert(success);
    auto [ii, ss] = _send.emplace(version, from_version);
    assert(ss);
    return version;
}

void CommunicatorHandle::send_transition(uint32_t version, MetaData& meta) {
    auto iter = _send.find(version);
    if (iter != _send.end()) {
        meta.version = iter->second;
        meta.link_mark = LinkMark::Info;
    } else {
        create_send_link(version);
        meta.version = version;
        meta.link_mark = LinkMark::Create;
    }
}

void CommunicatorHandle::receive_transition(MetaData& meta) {
    auto iter = _receive.find(meta.version);
    if (iter == _receive.end()) {
        meta.version = create_receive_link(meta.version);
        meta.link_mark = LinkMark::Create;
    } else {
        meta.version = iter->second;
    }
}

CommunicatorHandle::MessageEvent::MessageEvent(CommunicatorEvent::Type type, IdentityPtr id,
                                               const MetaData& meta, SerializablePtr data) :
    type(type), id(std::move(id)), meta(meta) {
    if (data && data->base_type() == (int) SerializableBaseType::Data) {
        variant = std::dynamic_pointer_cast<Data>(data);
    } else {
        variant = std::move(data);
    }
}
