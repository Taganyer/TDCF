//
// Created by taganyer on 25-5-15.
//
#pragma once

#include <tdcf/detail/NodeInformation.hpp>
#include <tdcf/node/agents/NodeAgent.hpp>


namespace tdcf {

    class Node : NoCopy {
    public:
        Node(IdentityPtr ip, CommunicatorPtr cp, ProcessorPtr pp, IdentityPtr root_id);

        virtual ~Node() = default;

        [[nodiscard]] StatusFlag handle_a_loop();

    protected:
        void join_in_cluster();

        StatusFlag agent_analysis_message(CommunicatorEvent& event);

        StatusFlag agent_handle_message(CommunicatorEvent& event);

        virtual StatusFlag active_events();

        virtual StatusFlag analysis_messages();

        virtual StatusFlag handle_messages();

        NodeInformation _info;

        NodeAgentPtr _agent;

    };

}
