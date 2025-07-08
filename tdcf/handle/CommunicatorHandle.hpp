//
// Created by taganyer on 25-6-29.
//
#pragma once

#include <map>
#include <set>
#include <tdcf/base/Version.hpp>
#include <tdcf/detail/EventProgress.hpp>
#include <tdcf/detail/MetaData.hpp>
#include <tdcf/detail/StatusFlag.hpp>
#include <tdcf/frame/Communicator.hpp>

namespace tdcf {

    class CommunicatorHandle {
    public:
        struct MessageEvent;

        explicit CommunicatorHandle(CommunicatorPtr ptr);

        ~CommunicatorHandle();

        void connect(const IdentityPtr& identity) const;

        [[nodiscard]] IdentityPtr accept() const;

        void disconnect(const IdentityPtr& id) const;

        uint32_t create_conversation_version();

        void close_conversation(uint32_t version);

        StatusFlag get_communicator_events();

        bool get_message(MessageEvent& message);

        StatusFlag send_message(const IdentityPtr& target, MetaData meta, SerializablePtr message);

        StatusFlag send_progress_message(uint32_t version, const IdentityPtr& target,
                                         MetaData meta, SerializablePtr message);

        StatusFlag send_delay_message(const IdentityPtr& target);

        [[nodiscard]] bool delayed_message(const IdentityPtr& target) const;

    private:
        using Key = std::pair<uint32_t, IdentityPtr>;

        struct Cmp {
            bool operator()(const Key& lhs, const Key& rhs) const {
                if (lhs.first != rhs.first) return lhs.first < rhs.first;
                if (lhs.second && rhs.second) return lhs.second->less_than(*rhs.second);
                return !lhs.second;
            };
        };

        CommunicatorPtr _communicator;

        using TransverterMap = std::map<Key, uint32_t, Cmp>;

        TransverterMap _receive, _send;

        using SendDelayMQ = std::map<IdentityPtr, std::queue<std::pair<MetaData, SerializablePtr>>,
                                     IdentityPtrLess>;

        SendDelayMQ _delay_queue;

        using MessageRQ = Communicator::EventQueue;

        MessageRQ _receive_queue;

        Version _version;

        StatusFlag send(const IdentityPtr& target, MetaData meta, SerializablePtr message);

        void create_send_link(uint32_t version, const IdentityPtr& to);

        uint32_t create_receive_link(uint32_t from_version, const IdentityPtr& from);

        void send_transition(uint32_t version, const IdentityPtr& to, MetaData& meta);

        bool receive_transition(const IdentityPtr& from, MetaData& meta) const;

    public:
        struct MessageEvent {
            CommunicatorEvent::Type type = CommunicatorEvent::Null;
            IdentityPtr id;
            MetaData meta;
            Variant variant;

            MessageEvent() = default;

            MessageEvent(CommunicatorEvent::Type type, IdentityPtr id,
                         const MetaData& meta, SerializablePtr data);
        };
    };

}
