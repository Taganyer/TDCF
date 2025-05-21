//
// Created by taganyer on 25-5-19.
//
#pragma once

#include <memory>
#include <vector>

#include "Identity.hpp"

namespace tdcf {

    class CommanderFlag {};

    // accept close send receive
    class CommanderEvent {};

    class Commander {
    public:
        using EventQueue = std::vector<CommanderEvent>;

        Commander() = default;

        virtual ~Commander() = default;

        virtual CommanderFlag connect(const Identity& id) = 0;

        virtual CommanderFlag accept(Identity& accepted_id) = 0;

        virtual CommanderFlag disconnect(const Identity& id) = 0;

        virtual CommanderFlag unblock_send(const Identity& id, const void *data, unsigned size) = 0;

        virtual CommanderFlag unblock_receive(Identity& id, void *buf, unsigned buf_size,
                                              unsigned& received_size) = 0;

        virtual CommanderFlag add_event(const CommanderEvent& event) = 0;

        virtual CommanderFlag remove_event(const CommanderEvent& event) = 0;

        virtual CommanderFlag get_alive_event(EventQueue& queue) = 0;

    };

    using CommanderPtr = std::shared_ptr<Commander>;

}
