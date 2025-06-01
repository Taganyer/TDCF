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

    enum class CommunicatorType {
        Null,
        SendMessage,

        MessageSendable,
        ReceivedMessage,
        ConnectRequest,
        DisconnectRequest,
    };

    struct CommunicatorEvent {
        CommunicatorType type = CommunicatorType::Null;

        IdentityPtr target_id;

        SerializablePtr message;
    };

    class Communicator : NoCopy {
    public:
        using EventQueue = std::queue<CommunicatorEvent>;

        Communicator() = default;

        virtual ~Communicator() = default;

        virtual StatusFlag connect(const IdentityPtr& target);

        virtual StatusFlag accept(const IdentityPtr& target);

        virtual StatusFlag disconnect(const IdentityPtr& id) = 0;

        virtual StatusFlag add_event(CommunicatorEvent event) = 0;

        virtual StatusFlag get_event(EventQueue& queue) = 0;

    };

    using CommunicatorPtr = std::shared_ptr<Communicator>;

}
