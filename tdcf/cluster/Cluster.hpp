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

        virtual void broadcast(ProcessingRulesPtr rule_ptr);

        virtual void scatter(ProcessingRulesPtr rule_ptr);

        virtual void reduce(ProcessingRulesPtr rule_ptr);

        virtual void all_gather(ProcessingRulesPtr rule_ptr);

        virtual void all_reduce(ProcessingRulesPtr rule_ptr);

        virtual void reduce_scatter(ProcessingRulesPtr rule_ptr);

        virtual void all_to_all(ProcessingRulesPtr rule_ptr);

    protected:
        StatusFlag active_events() override;

        StatusFlag analysis_messages() override;

        StatusFlag handle_messages() override;

        virtual StatusFlag handle_HTCEvents() = 0;

        // virtual StatusFlag handle_CTCEvents() = 0;

        MetaData _meta;

    };

}
