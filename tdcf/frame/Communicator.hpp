//
// Created by taganyer on 25-5-19.
//
#pragma once

#include <memory>
#include <queue>
#include <tdcf/base/NoCopy.hpp>
#include <tdcf/detail/Message.hpp>
#include <tdcf/frame/Identity.hpp>

namespace tdcf {

    enum class OperationFlag : uint8_t;

    struct CommunicatorEvent {
        enum Type : uint8_t {
            Null,
            MessageSendable,
            ReceivedMessage,
            ConnectRequest,
            DisconnectRequest,
        };

        Type type = Null;
        IdentityPtr id;
        MetaData meta;
        SerializablePtr data;

    };

    class Communicator : NoCopy {
    public:
        using EventQueue = std::queue<CommunicatorEvent>;

        Communicator() = default;

        virtual ~Communicator() = default;

        virtual bool connect(const IdentityPtr& target);

        virtual bool accept(const IdentityPtr& target);

        virtual bool disconnect(const IdentityPtr& id) = 0;

        virtual OperationFlag send_message(const IdentityPtr& id, const Message& message,
                                           const SerializablePtr& data) = 0;

        virtual OperationFlag get_events(EventQueue& queue) = 0;

    };

    using CommunicatorPtr = std::shared_ptr<Communicator>;

}
