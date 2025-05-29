//
// Created by taganyer on 25-5-15.
//
#pragma once

#include <tdcf/detail/NodeAgent.hpp>

namespace tdcf {

    class Node : NoCopy {
    public:
        Node(IdentityPtr idp, CommunicatorPtr cp, ProcessorPtr pp, InterpreterPtr inp);

        virtual ~Node() = default;

        [[nodiscard]] virtual StatusFlag join_in_cluster(const IdentityPtr& cluster_id);

        [[nodiscard]] virtual StatusFlag handle_a_loop();

    protected:
        NodeInformation _info;

        NodeAgentPtr _node_data;

    private:
        Communicator::EventQueue _events;

    };

}
