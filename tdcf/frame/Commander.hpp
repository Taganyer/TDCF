//
// Created by taganyer on 25-5-19.
//
#pragma once

#include <memory>
#include <vector>
#include <tdcf/base/NoCopy.hpp>
#include <tdcf/frame/Identity.hpp>
#include <tdcf/frame/StatusFlag.hpp>

namespace tdcf {

    // accept close send receive
    class CommanderEventMark {
    public:
        void set_none() {
            type = 0;
        };

        void set_send(bool on) {
            if (on) type |= 1;
            else type &= ~1;
        };

        void set_receive(bool on) {
            if (on) type |= (1 << 1);
            else type &= ~(1 << 1);
        };

        void set_connect() {
            type |= (1 << 2);
        };

        void set_disconnect() {
            type |= (1 << 3);
        };

        [[nodiscard]] bool is_none() const {
            return type == 0;
        };

        [[nodiscard]] bool can_send() const {
            return type & 1;
        };

        [[nodiscard]] bool can_receive() const {
            return type & (1 << 1);
        };

        [[nodiscard]] bool need_connect() const {
            return type & (1 << 2);
        }

        [[nodiscard]] bool need_disconnect() const {
            return type & (1 << 3);
        };

    private:
        int type = 0;
    };

    struct CommanderEvent {
        IdentityPtr id;

        CommanderEventMark mark;
    };

    class Commander : NoCopy {
    public:
        using EventQueue = std::vector<CommanderEvent>;

        Commander() = default;

        virtual ~Commander() = default;

        virtual StatusFlag connect_server(const IdentityPtr& id, SerializablePtr& server_data) = 0;

        virtual StatusFlag accept_client(IdentityPtr& accepted_id) = 0;

        virtual StatusFlag connect_client(const IdentityPtr& id, SerializablePtr& server_data) = 0;

        virtual StatusFlag disconnect(const IdentityPtr& id) = 0;

        virtual StatusFlag unblock_send(const IdentityPtr& id, const SerializablePtr& data_ptr) = 0;

        virtual StatusFlag unblock_receive(IdentityPtr& id, SerializablePtr& buffer_ptr) = 0;

        virtual StatusFlag add_event(const CommanderEvent& event) = 0;

        virtual StatusFlag remove_event(const CommanderEvent& event) = 0;

        virtual StatusFlag get_alive_event(EventQueue& queue) = 0;

    };

    using CommanderPtr = std::shared_ptr<Commander>;

}
