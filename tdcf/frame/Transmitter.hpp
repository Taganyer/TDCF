//
// Created by taganyer on 25-5-19.
//
#pragma once

#include <tdcf/frame/Data.hpp>
#include <tdcf/frame/Identity.hpp>

namespace tdcf {

    class TransmitterEventMark {
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

    struct TransmitterEvent {
        IdentityPtr id;

        TransmitterEventMark mark;
    };

    class Transmitter : NoCopy {
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
