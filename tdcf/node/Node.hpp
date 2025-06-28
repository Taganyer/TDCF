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

        virtual ~Node() { assert(!_node_agent_started); };

        virtual void start(unsigned cluster_size);

        [[nodiscard]] StatusFlag handle_a_loop();

        [[nodiscard]] bool node_agent_started() const { return _node_agent_started; };

    protected:
        void end_agent();

        virtual StatusFlag handle_message(CommunicatorEvent& event);

        StatusFlag handle_progress_task(NodeInformation::ProgressTask& task);

        StatusFlag active_communicator_events();

        StatusFlag active_processor_events();

        StatusFlag handle_communicator_events();

        StatusFlag handle_processor_events();

        bool _node_agent_started = false;

        bool _cluster_started = false;

        bool _cluster_closing = false;

        unsigned _cluster_events = 0;

        NodeInformation _info;

        NodeAgentPtr _agent;

    };

}
