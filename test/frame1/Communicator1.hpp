//
// Created by taganyer on 25-7-3.
//
#pragma once

#include <map>
#include <set>
#include <tdcf/frame/Communicator.hpp>
#include <tinyBackend/Base/Buffer/RingBuffer.hpp>
#include <tinyBackend/Base/Condition.hpp>


namespace test {

    struct CommShare {
        using Key = std::pair<uint32_t, uint32_t>;

        using Value = Base::RingBuffer;

        std::map<Key, Value> message;

        std::set<Key> connect;

        std::map<Key, bool> disconnect;

        Base::Mutex mutex;

        std::map<uint32_t, Base::Condition> conditions;

    };

    class Communicator1 : public tdcf::Communicator {
    public:
        static constexpr uint32_t receive_size = 1 << 12;

        Communicator1(uint32_t id, CommShare& share) : _id(id), _share(&share) {};

        bool connect(const tdcf::IdentityPtr& target) override;

        tdcf::IdentityPtr accept() override;

        bool disconnect(const tdcf::IdentityPtr& target) override;

        tdcf::OperationFlag send_message(const tdcf::IdentityPtr& target, const tdcf::Message& message,
                                         const tdcf::SerializablePtr& data) override;

        tdcf::OperationFlag get_events(EventQueue& queue) override;

    private:
        using Key = CommShare::Key;

        using Value = CommShare::Value;

        uint32_t _id;

        CommShare *_share;

        std::map<uint32_t, uint32_t> _delay;

        uint32_t get_messages(EventQueue& queue);

        static uint32_t get_message(EventQueue& queue, Base::RingBuffer& buf, tdcf::IdentityPtr& from);

        static tdcf::SerializablePtr get_data(tdcf::SerializableBaseType type, uint32_t size,
                                              Base::RingBuffer& buf);

        uint32_t check_delay(EventQueue& queue);

        uint32_t check_connect(Base::Lock<Base::Mutex>& l);

        uint32_t check_disconnect(EventQueue& queue);

    };

}
