//
// Created by taganyer on 25-5-15.
//
#pragma once

#include <tdcf/detail/NodeInformation.hpp>
#include <tdcf/node/agents/NodeAgent.hpp>


namespace tdcf {

    class Node : NoCopy {
    public:
        Node(IdentityPtr ip, CommunicatorPtr cp, ProcessorPtr pp);

        Node(IdentityPtr ip, CommunicatorPtr cp, ProcessorPtr pp, IdentityPtr root_id);

        virtual ~Node() = default;

        [[nodiscard]] StatusFlag handle_a_loop();

    protected:
        void join_in_cluster();

        virtual StatusFlag handle_message(CommunicatorEvent& event);

        virtual StatusFlag handle_progress_task(NodeInformation::ProgressTask& task);

        StatusFlag active_communicator_events();

        StatusFlag handle_communicator_events();

        StatusFlag active_processor_events();

        StatusFlag handle_processor_events();

        NodeInformation _info;

        NodeAgentPtr _agent;

    };

}
