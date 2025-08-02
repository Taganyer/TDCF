//
// Created by taganyer on 25-5-15.
//
#pragma once

#include <tdcf/handle/Handle.hpp>
#include <tdcf/node/agents/NodeAgent.hpp>


namespace tdcf {

    class Node : NoCopy {
    public:
        Node(IdentityPtr ip, CommunicatorPtr cp, ProcessorPtr pp);

        virtual ~Node();

        /// Cluster 不应调用
        void start_node();

        [[nodiscard]] StatusFlag handle_a_loop();

        [[nodiscard]] bool node_agent_started() const { return _node_agent_started; };

    private:
        MetaData get_agent();

    protected:
        void end_agent();

        virtual StatusFlag handle_message(Handle::MessageEvent& event);

        StatusFlag handle_progress_task(Handle::ProgressTask& task);

        StatusFlag active_communicator_events();

        StatusFlag active_processor_events();

        StatusFlag handle_communicator_events();

        StatusFlag handle_processor_events();

        bool _node_agent_started = false;

        bool _cluster_staring = false;

        bool _cluster_started = false;

        bool _cluster_closing = false;

        Handle _handle;

        NodeAgentPtr _agent;

    };

}
