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

using namespace test;

using namespace tdcf;

bool Communicator1::connect(const IdentityPtr& target) {
    uint32_t id = static_cast<Identity1&>(target).id();
    Key k { _id, id };
    auto [iter, success] = _share->message.emplace(k, Value(receive_size));
    assert(success);
    _share->connect.emplace(id, _id);
    return true;
}

bool Communicator1::accept(const IdentityPtr& target) {
    uint32_t id = static_cast<Identity1&>(target).id();
    Key k { _id, id };
    auto [iter, success] = _share->message.emplace(k, Value(receive_size));
    assert(success);
    return true;
}

bool Communicator1::disconnect(const IdentityPtr& target) {
    uint32_t id = static_cast<Identity1&>(target).id();
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
    uint32_t id = static_cast<Identity1&>(target).id();
    Key key(id, _id);
    auto iter = _share->message.find(key);
    assert(iter != _share->message.end());
    auto size = message.serialize_size() +
        (sizeof(SerializableType) + sizeof(uint32_t)) +
        (data ? data->serialize_size() : 0);
    assert(size <= receive_size);
    if (iter->second.writable_len() < size) {
        assert(_delay[id] == 0);
        _delay[id] = size;
        return OperationFlag::FurtherWaiting;
    }
    std::string str(size, '\0');
    auto ptr1 = (uint8_t *) str.data();
    ptr1[0] = (uint8_t) (data ? data->base_type() : 0);
    auto ptr2 = (uint32_t *) (str.data() + 1);
    ptr2[0] = data ? data->serialize_size() : 0;
    message.serialize(str.data() + 1 + 4, size);
    data->serialize(str.data() + 1 + 4 + message.serialize_size(), size);
    uint32_t writen = iter->second.write(str.data(), str.size());
    assert(writen == size);
    return OperationFlag::Success;
}

OperationFlag Communicator1::get_events(EventQueue& queue) {
    uint32_t get = 0;
    get += get_messages(queue);
    get += check_delay(queue);
    get += check_connect(queue);
    get += check_disconnect(queue);
    if (get == 0) return OperationFlag::FurtherWaiting;
    return OperationFlag::Success;
}

uint32_t Communicator1::get_messages(EventQueue& queue) {
    Key key(_id, 0);
    auto iter = _share->message.find(key);
    uint32_t get = 0;
    while (iter != _share->message.end() && iter->first.first == _id) {
        auto& buf = iter->second;
        IdentityPtr from = std::make_shared<Identity1>(iter->first.second);
        get += get_message(queue, buf, from);
        ++iter;
    }
    return get;
}

uint32_t Communicator1::get_message(EventQueue& queue, Base::RingBuffer& buf, IdentityPtr& from) {
    uint32_t get = 0;
    while (buf.readable_len() != 0) {
        SerializableBaseTypes type = SerializableBaseTypes::Null;
        auto read = buf.read(&type, 1);
        assert(read == 1);

        uint32_t size = 0;
        read = buf.read(&size, 4);
        assert(read == 4);

        Message meta;
        read = buf.read(&meta, meta.serialize_size());
        assert(read == meta.serialize_size());

        CommunicatorEvent event { CommunicatorEvent::ReceivedMessage, from,
                                  meta.meta_data, get_data(type, size, meta, buf) };
        queue.emplace(std::move(event));
        ++get;
    }
    return get;
}

SerializablePtr Communicator1::get_data(SerializableBaseTypes type, uint32_t size,
                                        Message& meta, Base::RingBuffer& buf) {
    std::string str(size, '\0');
    auto read = buf.read(str.data(), size);
    assert(read == size);
    bool success = false;
    switch (type) {
        case SerializableBaseTypes::Null:
            return nullptr;
        case SerializableBaseTypes::Message:
            auto message = std::make_shared<Message>();
            success = message->deserialize(str.data(), str.size());
            assert(success);
            return message;
        case SerializableBaseTypes::Identity:
            auto identity = std::make_shared<Identity1>();
            success = identity->deserialize(str.data(), str.size());
            assert(success);
            return identity;
        case SerializableBaseTypes::NodeAgent:
            SerializablePtr ptr;
            auto s = NodeAgent::deserialize_NodeAgent(meta.meta_data, ptr, str.data(), str.size());
            assert(s == StatusFlag::Success);
            return ptr;
        case SerializableBaseTypes::Data:
            auto data = std::make_shared<Data1>();
            success = data->deserialize(str.data(), str.size());
            assert(success);
            return data;
        case SerializableBaseTypes::ProcessingRules:
            auto rules = std::make_shared<ProcessingRules1>();
            success = rules->deserialize(str.data(), str.size());
            assert(success);
            return rules;
        default:
            TDCF_RAISE_ERROR(error type)
    }
}

uint32_t Communicator1::check_delay(EventQueue& queue) {
    uint32_t get = 0;
    for (auto& [id, size] : _delay) {
        Key k(id, _id);
        auto iter = _share->message.find(k);
        assert(iter != _share->message.end());
        if (iter->second.writable_len() < size) continue;
        CommunicatorEvent event {
            CommunicatorEvent::MessageSendable, std::make_shared<Identity1>(id),
            MetaData(), nullptr
        };
        queue.emplace(std::move(event));
        size = 0;
        ++get;
    }
    return get;
}

uint32_t Communicator1::check_connect(EventQueue& queue) {
    uint32_t get = 0;
    Key key(_id, 0);
    auto iter = _share->connect.lower_bound(key);
    while (iter != _share->connect.end() && iter->first == _id) {
        CommunicatorEvent event {
            CommunicatorEvent::ConnectRequest, std::make_shared<Identity1>(iter->second),
            MetaData(), nullptr
        };
        queue.emplace(std::move(event));
        ++get;
        iter = _share->connect.erase(iter);
    }
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
    return get;
}
