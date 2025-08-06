//
// Created by taganyer on 25-6-29.
//
#pragma once

#include <map>
#include <tdcf/base/Version.hpp>
#include <tdcf/detail/EventProgress.hpp>
#include <tdcf/detail/MetaData.hpp>
#include <tdcf/detail/StatusFlag.hpp>
#include <tdcf/frame/Communicator.hpp>

namespace tdcf {

    class CommunicatorHandle {
    public:
        struct MessageEvent;

        explicit CommunicatorHandle(CommunicatorPtr ptr, Identity::Uid uid);

        ~CommunicatorHandle();

        void connect(const IdentityPtr& identity) const;

        [[nodiscard]] IdentityPtr accept() const;

        void disconnect(const IdentityPtr& id) const;

        uint32_t create_progress_version();

        void close_progress(uint32_t version);

        StatusFlag get_communicator_events();

        bool get_message(MessageEvent& message);

        void waiting_for_message(MessageEvent& event);

        StatusFlag send_message(const IdentityPtr& target, MetaData meta, SerializablePtr message);

        StatusFlag send_progress_message(uint32_t version, const IdentityPtr& target,
                                         MetaData meta, SerializablePtr message);

        StatusFlag send_delay_message(const IdentityPtr& target);

        [[nodiscard]] bool delayed_message(const IdentityPtr& target) const;

    private:
        CommunicatorPtr _communicator;

        using Uid = Identity::Uid;

        using ID = std::pair<Uid, uint32_t>;

        struct IDHash {
            std::size_t operator()(const ID& id) const {
                return std::hash<unsigned int>{}(id.first) ^
                       std::hash<unsigned int>{}(id.second) << 1;
            }
        };

        using RTransverterMap = std::unordered_map<ID, uint32_t, IDHash>;

        using STransverterMap = std::unordered_map<uint32_t, ID>;

        RTransverterMap _receive;

        STransverterMap _send;

        using SendDelayMQ = std::map<IdentityPtr, std::queue<std::pair<MetaData, SerializablePtr>>,
                                     IdentityPtrLess>;

        SendDelayMQ _delay_queue;

        using MessageRQ = Communicator::EventQueue;

        MessageRQ _receive_queue;

        Identity::Uid _uid;

        Version _version;

        StatusFlag send(const IdentityPtr& target, MetaData meta, SerializablePtr message);

        void create_send_link(uint32_t version);

        uint32_t create_receive_link(ID id);

        void send_transition(uint32_t version, MetaData& meta);

        void receive_transition(MetaData& meta);

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
