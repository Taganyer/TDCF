//
// Created by taganyer on 25-5-19.
//
#pragma once

#include <memory>
#include <queue>
#include <tdcf/base/NoCopy.hpp>
#include <tdcf/frame/Identity.hpp>
#include <tdcf/detail/StatusFlag.hpp>

namespace tdcf {

    struct CommunicatorEvent {
        enum Type {
            Null,
            MessageSendable,
            ReceivedMessage,
            ConnectRequest,
            DisconnectRequest,
        };

        Type type = Null;

        IdentityPtr target_id;

        MetaData meta;

        SerializablePtr data;
    };

    class Communicator : NoCopy {
    public:
        using EventQueue = std::queue<CommunicatorEvent>;

        Communicator() = default;

        virtual ~Communicator() = default;

        virtual StatusFlag connect(const IdentityPtr& target);

        virtual StatusFlag accept(const IdentityPtr& target);

        virtual StatusFlag disconnect(const IdentityPtr& id) = 0;

        virtual StatusFlag send_message(const IdentityPtr& id, const Message& message,
                                        const SerializablePtr& data) = 0;

        virtual StatusFlag get_events(EventQueue& queue) = 0;

    };

    using CommunicatorPtr = std::shared_ptr<Communicator>;

}
