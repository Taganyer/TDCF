//
// Created by taganyer on 25-6-29.
//
#pragma once

#include <map>
#include <tdcf/base/Version.hpp>
#include <tdcf/detail/MetaData.hpp>
#include <tdcf/detail/StatusFlag.hpp>
#include <tdcf/frame/Communicator.hpp>

namespace tdcf {

    class CommunicatorHandle {
    public:
        struct MessageEvent;

        explicit CommunicatorHandle(IdentityPtr self, CommunicatorPtr ptr) :
            _self_id(std::move(self)), _communicator(std::move(ptr)) {};

        void connect(const IdentityPtr& identity);

        void accept(const IdentityPtr& identity);

        void disconnect(uint32_t id);

        void start_communicator_handle();

        uint32_t create_conversation_version();

        void close_conversation(uint32_t version);

        StatusFlag get_communicator_events();

        MessageEvent get_message();

        StatusFlag send_message(uint32_t version, uint32_t target,
                                MetaData meta, SerializablePtr message);

        StatusFlag send_delay_message(uint32_t target);

        bool delayed_message(uint32_t target);

        [[nodiscard]] uint32_t id_list_size() const;

        [[nodiscard]] uint32_t communicator_events_size() const;

        [[nodiscard]] uint32_t get_identity_serial(const IdentityPtr& identity) const;

    private:
        IdentityPtr _self_id;

        CommunicatorPtr _communicator;

        using IdentityList = std::vector<IdentityPtr>;

        IdentityList _id_list;

        using TransverterMap = std::map<std::pair<uint32_t, uint32_t>, uint32_t>;

        TransverterMap _receive, _send;

        using SendDelayMQ = std::map<uint32_t, std::queue<std::pair<MetaData, SerializablePtr>>>;

        SendDelayMQ _delay_queue;

        using MessageRQ = Communicator::EventQueue;

        MessageRQ _receive_queue;

        Version _version;

        void send_transition(uint32_t version, uint32_t to, MetaData& meta);

        uint32_t receive_transition(uint32_t from, const MetaData& meta);

    public:
        struct MessageEvent {
            uint32_t from_id;
            CommunicatorEvent::Type type;
            MetaData meta;
            SerializablePtr data;
        };

    };

}
