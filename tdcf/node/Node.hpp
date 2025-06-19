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

        virtual ~Node() { assert(!_start); };

        virtual void start(unsigned cluster_size);

        [[nodiscard]] StatusFlag handle_a_loop();

        [[nodiscard]] bool parent_cluster_started() const { return _start; };

    protected:
        virtual StatusFlag handle_message(CommunicatorEvent& event);

        StatusFlag handle_progress_task(NodeInformation::ProgressTask& task);

        StatusFlag active_communicator_events();

        StatusFlag active_processor_events();

        StatusFlag handle_communicator_events();

        StatusFlag handle_processor_events();

        bool _start = false;

        bool _cluster_start = false;

        unsigned _cluster_events = 0;

        NodeInformation _info;

        NodeAgentPtr _agent;

    };

}
