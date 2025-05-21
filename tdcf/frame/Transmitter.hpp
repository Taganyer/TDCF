//
// Created by taganyer on 25-5-19.
//
#pragma once

#include "Data.hpp"
#include "Identity.hpp"

namespace tdcf {

    class TransmitterFlag {};

    // accept close send receive
    class TransmitterEvent {};

    class Transmitter {
    public:
        using EventQueue = std::vector<TransmitterEvent>;

        Transmitter() = default;

        virtual ~Transmitter() = default;

        virtual TransmitterFlag connect(const Identity& id) = 0;

        virtual TransmitterFlag accept(Identity& accepted_id) = 0;

        virtual TransmitterFlag disconnect(const Identity& id) = 0;

        virtual TransmitterFlag unblock_send(const Identity& id, Data& data) = 0;

        virtual TransmitterFlag unblock_receive(Identity& id, Data& buf) = 0;

        virtual TransmitterFlag add_event(const TransmitterEvent& event) = 0;

        virtual TransmitterFlag remove_event(const TransmitterEvent& event) = 0;

        virtual TransmitterFlag get_alive_event(EventQueue& queue) = 0;

    };

    using TransmitterPtr = std::shared_ptr<Transmitter>;

}
