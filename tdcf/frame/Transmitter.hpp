//
// Created by taganyer on 25-5-19.
//
#pragma once

#include <tinyBackend/Base/Detail/NoCopy.hpp>

#include "Data.hpp"
#include "Identity.hpp"

namespace tdcf {

    // accept close send receive
    class TransmitterEvent {};

    class Transmitter : Base::NoCopy {
    public:
        using EventQueue = std::vector<TransmitterEvent>;

        Transmitter() = default;

        virtual ~Transmitter() = default;

        virtual StatusFlag connect_server(const IdentityPtr& id_ptr) = 0;

        virtual StatusFlag accept_client(IdentityPtr& accepted_id_ptr) = 0;

        virtual StatusFlag disconnect(const IdentityPtr& id_ptr) = 0;

        virtual StatusFlag unblock_send(const IdentityPtr& id_ptr, const DataPtr& data) = 0;

        virtual StatusFlag unblock_receive(const IdentityPtr &id_ptr, DataPtr& buffer_ptr) = 0;

        virtual StatusFlag add_event(const TransmitterEvent& event) = 0;

        virtual StatusFlag remove_event(const TransmitterEvent& event) = 0;

        virtual StatusFlag get_alive_event(EventQueue& queue) = 0;

    };

    using TransmitterPtr = std::shared_ptr<Transmitter>;

}
