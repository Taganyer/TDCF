//
// Created by taganyer on 25-5-15.
//
#pragma once

#include <tdcf/node/Node.hpp>

namespace tdcf {

    class Cluster : public Node {
    public:
        Cluster(IdentityPtr idp, CommunicatorPtr cp, ProcessorPtr pp, InterpreterPtr inp) :
            Node(std::move(idp), std::move(cp), std::move(pp), std::move(inp)) {};

        virtual void broadcast(ProcessingRulesPtr rule_ptr);

        virtual void scatter(ProcessingRulesPtr rule_ptr);

        virtual void reduce(ProcessingRulesPtr rule_ptr);

        virtual void all_gather(ProcessingRulesPtr rule_ptr);

        virtual void all_reduce(ProcessingRulesPtr rule_ptr);

        virtual void reduce_scatter(ProcessingRulesPtr rule_ptr);

        virtual void all_to_all(ProcessingRulesPtr rule_ptr);

    protected:
        struct ClusterEvent : InternalEvent {
            ProcessingRulesPtr rule;

            explicit ClusterEvent(EventType t, ProcessingRulesPtr rp) :
                InternalEvent(t), rule(std::move(rp)) {};
        };

        using HTCEventQueue = std::queue<HTCEventPtr>;

        HTCEventQueue _self_queue;

        friend class NodeAgent;

    };

}
