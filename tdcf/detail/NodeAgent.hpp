//
// Created by taganyer on 25-5-22.
//
#pragma once

#include <queue>
#include <tdcf/detail/InternalEvent.hpp>
#include <tdcf/frame/Commander.hpp>
#include <tdcf/frame/Identity.hpp>
#include <tdcf/frame/Interpreter.hpp>
#include <tdcf/frame/Transmitter.hpp>

namespace tdcf {

    struct NodeInformation {
        IdentityPtr id;

        TransmitterPtr transmitter;

        CommanderPtr commander;

        ProcessorPtr processor;

        InterpreterPtr interpreter;

        NodeInformation() = default;

        NodeInformation(IdentityPtr idp, TransmitterPtr tp, CommanderPtr cp,
                        ProcessorPtr pp, InterpreterPtr inp) :
            id(std::move(idp)),
            transmitter(std::move(tp)),
            commander(std::move(cp)),
            processor(std::move(pp)),
            interpreter(std::move(inp)) {};

        [[nodiscard]] bool check() const {
            return id && transmitter && commander && processor && interpreter;
        };

    };


    class Cluster;

    class NodeAgent : public Serializable {
    public:
        NodeAgent() = default;

        ~NodeAgent() override = default;

        [[nodiscard]] SerializableType base_type() const final {
            return static_cast<SerializableType>(SerializableBaseTypes::NodeAgent);
        };

        virtual StatusFlag handle_node_loop(NodeInformation& info) = 0;

        virtual StatusFlag handle_cluster_loop(NodeInformation& info, Cluster& self) = 0;

        IdentityPtr cluster_id;

    protected:
        using EventQueue = std::queue<InternalEventPtr>;

        EventQueue _event_queue;

    };

    using NodeAgentPtr = std::shared_ptr<NodeAgent>;

    enum class NodeDataTypes {
        StarNode,
    };

}
