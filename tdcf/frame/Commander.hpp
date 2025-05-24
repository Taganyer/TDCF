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
    class CommanderEvent {};

    class Commander : NoCopy {
    public:
        using EventQueue = std::vector<CommanderEvent>;

        Commander() = default;

        virtual ~Commander() = default;

        virtual StatusFlag connect_server(const IdentityPtr& id, SerializablePtr& server_data) = 0;

        virtual StatusFlag accept_server(IdentityPtr& accepted_id) = 0;

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
