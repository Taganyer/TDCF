//
// Created by taganyer on 25-5-22.
//
#pragma once

#include <queue>
#include <tdcf/detail/InternalEvent.hpp>
#include <tdcf/frame/Communicator.hpp>
#include <tdcf/frame/Identity.hpp>
#include <tdcf/frame/Interpreter.hpp>

namespace tdcf {

    struct NodeInformation {
        IdentityPtr id;

        CommunicatorPtr commander;

        ProcessorPtr processor;

        InterpreterPtr interpreter;

        NodeInformation() = default;

        NodeInformation(IdentityPtr idp, CommunicatorPtr cp,
                        ProcessorPtr pp, InterpreterPtr inp) :
            id(std::move(idp)),
            commander(std::move(cp)),
            processor(std::move(pp)),
            interpreter(std::move(inp)) {};

        [[nodiscard]] bool check() const {
            return id && commander && processor && interpreter;
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

        virtual StatusFlag handle_node_event(NodeInformation& info, CommunicatorEvent& event) = 0;

        virtual StatusFlag handle_cluster_event(NodeInformation& info, CommunicatorEvent& event) = 0;

        IdentityPtr cluster_id;

    };

    using NodeAgentPtr = std::shared_ptr<NodeAgent>;

    enum class NodeDataTypes {
        StarNode,
    };

}
