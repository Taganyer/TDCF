//
// Created by taganyer on 25-7-3.
//

#include <string>
#include <tdcf/base/Errors.hpp>
#include <tdcf/detail/StatusFlag.hpp>
#include <tdcf/node/agents/NodeAgent.hpp>
#include <test/frame1/Communicator1.hpp>
#include <test/frame1/Data1.hpp>
#include <test/frame1/Identity1.hpp>
#include <test/frame1/ProcessingRules1.hpp>
#include <tinyBackend/Base/Time/TimeInterval.hpp>

using namespace test;

using namespace tdcf;

using namespace Base;

bool Communicator1::connect(const IdentityPtr& target) {
    Lock l(_share->mutex);
    uint32_t id = static_cast<Identity1&>(*target).guid();
    Key k1 { id, _id }, k2 { _id, id };
    _share->connect.emplace(k1);
    auto [iter, success] = _share->message.emplace(k2, Value(receive_size));
    assert(success);
    _share->conditions[id].notify_one();
    _share->conditions[_id].wait(l, [this, k1] {
        return _share->message.find(k1) != _share->message.end();
    });
    T_INFO << __FUNCTION__ << " " << _id << "----" << id << " success";
    return true;
}

IdentityPtr Communicator1::accept() {
    Lock l(_share->mutex);
    uint32_t id = check_connect(l);
    Key k { _id, id };
    auto [iter, success] = _share->message.emplace(k, Value(receive_size));
    assert(success);
    _share->conditions[id].notify_one();
    T_INFO << __FUNCTION__ << " " << _id << "----" << id << " success";
    return std::make_shared<Identity1>(id);
}

bool Communicator1::disconnect(const IdentityPtr& target) {
    Lock l(_share->mutex);
    uint32_t id = static_cast<Identity1&>(*target).guid();
    Key k { _id, id };
    auto size = _share->message.erase(k);
    assert(size == 1);
    size = _share->disconnect.erase(k);
    if (!size) {
        auto [iter, success] = _share->disconnect.emplace(id, _id);
        assert(success);
    }
    return true;
}

OperationFlag Communicator1::send_message(const IdentityPtr& target,
                                          const Message& message,
                                          const SerializablePtr& data) {
    Lock l(_share->mutex);
    uint32_t id = static_cast<Identity1&>(*target).guid();
    assert(_id != id);
    Key key(id, _id);
    auto iter = _share->message.find(key);
    assert(iter != _share->message.end());

    auto size = sizeof(uint8_t) + sizeof(uint32_t) +
        message.serialize_size() +
        (data ? data->serialize_size() : 0);
    assert(size <= receive_size);

    if (iter->second.writable_len() < size) {
        assert(_delay[id] == 0);
        _delay[id] = size;
        return OperationFlag::FurtherWaiting;
    }

    std::string str(size, '\0');
    auto ptr1 = (uint8_t *) str.data();
    *ptr1 = (uint8_t) (data ? data->base_type() : 0);
    auto ptr2 = (uint32_t *) (str.data() + 1);
    *ptr2 = data ? data->serialize_size() : 0;
    message.serialize(str.data() + 1 + 4, size);
    if (data) {
        data->serialize(str.data() + 1 + 4 + message.serialize_size(), size);
    }
    uint32_t writen = iter->second.write(str.data(), str.size());
    assert(writen == size);
    assert(iter->second.readable_len() >= size);
    T_INFO << __FUNCTION__ << " " << _id << "----" << id << " operation: "
            << operation_type_name(message.meta_data.operation_type)
            << " base type: " << serializable_base_type_name(
            data ? (SerializableBaseType) data->base_type() : SerializableBaseType::Null);

    _share->conditions[id].notify_one();
    return OperationFlag::Success;
}

OperationFlag Communicator1::get_events(EventQueue& queue) {
    Lock l(_share->mutex);
    bool success = _share->conditions[_id].wait_for(l, 500_ms, [this, &queue] {
        uint32_t get = 0;
        get += get_messages(queue);
        get += check_delay(queue);
        get += check_disconnect(queue);
        return get != 0;
    });
    if (!success) return OperationFlag::FurtherWaiting;
    return OperationFlag::Success;
}

uint32_t Communicator1::get_messages(EventQueue& queue) {
    Key key(_id, 0);
    auto iter = _share->message.lower_bound(key);
    uint32_t get = 0;
    while (iter != _share->message.end() && iter->first.first == _id) {
        auto& buf = iter->second;
        IdentityPtr from = std::make_shared<Identity1>(iter->first.second);
        uint32_t t = get_message(queue, buf, from);
        if (t) {
            T_INFO << _id << " get " << t << " messages from " << iter->first.second;
        }
        get += t;
        ++iter;
    }
    // T_INFO << "Communicator1 " << _id << " get_messages: " << get;
    return get;
}

uint32_t Communicator1::get_message(EventQueue& queue, RingBuffer& buf, IdentityPtr& from) {
    uint32_t get = 0;
    while (buf.readable_len() != 0) {
        auto len = buf.readable_len();
        SerializableBaseType type = SerializableBaseType::Null;
        auto read = buf.read(&type, 1);
        assert(read == 1);

        uint32_t size = 0;
        read = buf.read(&size, 4);
        assert(read == 4);

        Message meta;
        std::string str(meta.serialize_size(), '\0');
        read = buf.read(str.data(), str.size());
        assert(read == meta.serialize_size());
        meta.deserialize(str.data(), str.size());

        CommunicatorEvent event { CommunicatorEvent::ReceivedMessage, from,
                                  meta.meta_data, get_data(type, size, meta, buf) };
        queue.emplace(std::move(event));
        ++get;
    }
    return get;
}

SerializablePtr Communicator1::get_data(SerializableBaseType type, uint32_t size,
                                        Message& meta, RingBuffer& buf) {
    std::string str(size, '\0');
    auto read = buf.read(str.data(), size);
    assert(read == size);
    bool success = false;
    switch (type) {
        case SerializableBaseType::Null:
            return nullptr;
        case SerializableBaseType::Message: {
            auto message = std::make_shared<Message>();
            success = message->deserialize(str.data(), str.size());
            assert(success);
            return message;
        }
        case SerializableBaseType::Identity: {
            auto identity = std::make_shared<Identity1>();
            success = identity->deserialize(str.data(), str.size());
            assert(success);
            return identity;
        }
        case SerializableBaseType::NodeAgent: {
            SerializablePtr ptr;
            auto s = NodeAgent::deserialize_NodeAgent(meta.meta_data, ptr, str.data(), str.size());
            assert(s == StatusFlag::Success);
            return ptr;
        }
        case SerializableBaseType::Data: {
            auto data = std::make_shared<Data1>();
            success = data->deserialize(str.data(), str.size());
            assert(success);
            return std::static_pointer_cast<Data>(data);
        }
        case SerializableBaseType::ProcessingRules: {
            auto rules = std::make_shared<ProcessingRules1>();
            success = rules->deserialize(str.data(), str.size());
            assert(success);
            return rules;
        }
    }
    TDCF_RAISE_ERROR(error type)
}

uint32_t Communicator1::check_delay(EventQueue& queue) {
    uint32_t get = 0;
    for (auto& [id, size] : _delay) {
        Key k(id, _id);
        auto iter = _share->message.find(k);
        assert(iter != _share->message.end());
        if (size == 0 || iter->second.writable_len() < size) continue;
        CommunicatorEvent event {
            CommunicatorEvent::MessageSendable, std::make_shared<Identity1>(id),
            MetaData(), nullptr
        };
        queue.emplace(std::move(event));
        size = 0;
        ++get;
    }
    // T_INFO << "Communicator1 " << _id << " check_delay: " << get;
    return get;
}

uint32_t Communicator1::check_connect(Lock<Mutex>& l) {
    uint32_t get = 0;
    _share->conditions[_id].wait(l, [this, &get] {
        Key key(_id, 0);
        auto iter = _share->connect.lower_bound(key);
        if (iter != _share->connect.end() && iter->first == _id) {
            get = iter->second;
            _share->connect.erase(iter);
            return true;
        }
        return false;
    });
    return get;
}

uint32_t Communicator1::check_disconnect(EventQueue& queue) {
    uint32_t get = 0;
    Key key(_id, 0);
    auto iter = _share->disconnect.lower_bound(key);
    while (iter != _share->disconnect.end() && iter->first == _id) {
        CommunicatorEvent event {
            CommunicatorEvent::DisconnectRequest, std::make_shared<Identity1>(iter->second),
            MetaData(), nullptr
        };
        queue.emplace(std::move(event));
        ++get;
        ++iter;
    }
    // T_INFO << "Communicator1 " << _id << " check_disconnect: " << get;
    return get;
}
