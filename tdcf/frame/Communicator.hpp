//
// Created by taganyer on 25-5-19.
//
#pragma once

#include <memory>
#include <vector>
#include <tdcf/base/NoCopy.hpp>
#include <tdcf/frame/Identity.hpp>
#include <tdcf/base/StatusFlag.hpp>

namespace tdcf {

    // accept close send receive
    class CommunicatorEventMark {
    public:
        void set_none() {
            type = 0;
        };

        void set_send_message(bool on) {
            if (on) type |= 1;
            else type &= ~1;
        };

        void set_receive_message(bool on) {
            if (on) type |= (1 << 1);
            else type &= ~(1 << 1);
        };

        void set_send_data(bool on) {
            if (on) type |= (1 << 2);
            else type &= ~(1 << 2);
        };

        void set_receive_data(bool on) {
            if (on) type |= (1 << 3);
            else type &= ~(1 << 3);
        };

        void set_connect() {
            type |= (1 << 4);
        };

        void set_disconnect() {
            type |= (1 << 5);
        };

        [[nodiscard]] bool is_none() const {
            return type == 0;
        };

        [[nodiscard]] bool can_send_message() const {
            return type & 1;
        };

        [[nodiscard]] bool can_receive_message() const {
            return type & (1 << 1);
        };

        [[nodiscard]] bool can_send_data() const {
            return type & (1 << 2);
        };

        [[nodiscard]] bool can_receive_data() const {
            return type & (1 << 3);
        };

        [[nodiscard]] bool need_connect() const {
            return type & (1 << 4);
        }

        [[nodiscard]] bool need_disconnect() const {
            return type & (1 << 5);
        };

    private:
        int type = 0;
    };

    struct CommunicatorEvent {
        IdentityPtr id;

        CommunicatorEventMark mark;
    };

    class Communicator : NoCopy {
    public:
        using EventQueue = std::vector<CommunicatorEvent>;

        Communicator() = default;

        virtual ~Communicator() = default;

        virtual StatusFlag connect_server(const IdentityPtr& sever, SerializablePtr& server_data) = 0;

        virtual StatusFlag accept_client(IdentityPtr& client) = 0;

        virtual StatusFlag connect_client(const IdentityPtr& client, SerializablePtr& server_data) = 0;

        virtual StatusFlag disconnect(const IdentityPtr& id) = 0;

        virtual StatusFlag unblock_send(const IdentityPtr& dest, const SerializablePtr& message) = 0;

        virtual StatusFlag unblock_receive(IdentityPtr& id, SerializablePtr& message_buffer) = 0;

        virtual StatusFlag unblock_send(const IdentityPtr& dest, const DataPtr& data) = 0;

        virtual StatusFlag unblock_receive(const IdentityPtr &id, DataPtr& data_buffer) = 0;

        virtual StatusFlag add_event(const CommunicatorEvent& event) = 0;

        virtual StatusFlag remove_event(const CommunicatorEvent& event) = 0;

        virtual StatusFlag get_alive_event(EventQueue& queue) = 0;

    };

    using CommunicatorPtr = std::shared_ptr<Communicator>;

}
