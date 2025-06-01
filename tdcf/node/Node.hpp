//
// Created by taganyer on 25-5-15.
//
#pragma once

#include <map>
#include <tdcf/detail/InternalEvent.hpp>
#include <tdcf/detail/NodeAgent.hpp>
#include <tdcf/frame/Communicator.hpp>
#include <tdcf/frame/Identity.hpp>
#include <tdcf/frame/Processor.hpp>

namespace tdcf {

    struct NodeInformation {
        IdentityPtr id;

        CommunicatorPtr communicator;

        ProcessorPtr processor;

        IdentityPtr root_id;

        NodeInformation() = default;

        NodeInformation(IdentityPtr ip, CommunicatorPtr cp, ProcessorPtr pp) :
            id(std::move(ip)),
            communicator(std::move(cp)),
            processor(std::move(pp)) {};

        NodeInformation(IdentityPtr ip, CommunicatorPtr cp, ProcessorPtr pp, IdentityPtr cluster) :
            id(std::move(ip)),
            communicator(std::move(cp)),
            processor(std::move(pp)),
            root_id(std::move(cluster)) {};

        [[nodiscard]] bool check() const {
            return id && communicator && processor;
        };

        using SendDelayMap = std::map<IdentityPtr, std::queue<SerializablePtr>, IdentityPtrLess>;

        SendDelayMap send_delay;

        StatusFlag send_message(const IdentityPtr& id, SerializablePtr message);

        StatusFlag send_delay_message(const IdentityPtr& id);

    };

    struct NodeEventData {
        using ReceivedQueue = Communicator::EventQueue;

        using InternalEventsMap = std::map<MetaData, InternalEventPtr>;

        using InternalEventsMapIter = InternalEventsMap::iterator;

        ReceivedQueue message_queue;

        InternalEventsMap internal_events;

    };

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

        NodeEventData _data;

        NodeAgentPtr _agent;

    };

}
