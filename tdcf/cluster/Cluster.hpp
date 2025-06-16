//
// Created by taganyer on 25-5-15.
//
#pragma once

#include <tdcf/node/Node.hpp>
#include <utility>

namespace tdcf {

    class Cluster : public Node {
    public:
        Cluster(IdentityPtr ip, CommunicatorPtr cp, ProcessorPtr pp, IdentityPtr root_id) :
            Node(std::move(ip), std::move(cp), std::move(pp), std::move(root_id)) {};

        virtual void broadcast(ProcessingRulesPtr rule_ptr) = 0;

        virtual void scatter(ProcessingRulesPtr rule_ptr) = 0;

        virtual void reduce(ProcessingRulesPtr rule_ptr) = 0;

        virtual void all_gather(ProcessingRulesPtr rule_ptr) = 0;

        virtual void all_reduce(ProcessingRulesPtr rule_ptr) = 0;

        virtual void reduce_scatter(ProcessingRulesPtr rule_ptr) = 0;

        virtual void all_to_all(ProcessingRulesPtr rule_ptr) = 0;

    };

}
