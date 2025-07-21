//
// Created by taganyer on 25-5-15.
//
#pragma once

#include <set>
#include <tdcf/node/Node.hpp>

namespace tdcf {

    class Cluster : public Node {
    public:
        using IdentitySet = std::set<IdentityPtr, IdentityPtrLess>;

        Cluster(IdentityPtr ip, CommunicatorPtr cp, ProcessorPtr pp);

        ~Cluster() override;

        void start_cluster(const IdentitySet& child_nodes, bool as_child_node);

        StatusFlag end_cluster();

        virtual StatusFlag broadcast(ProcessingRulesPtr rule_ptr) = 0;

        virtual StatusFlag scatter(ProcessingRulesPtr rule_ptr) = 0;

        virtual StatusFlag reduce(ProcessingRulesPtr rule_ptr) = 0;

        virtual StatusFlag all_reduce(ProcessingRulesPtr rule_ptr) = 0;

        virtual StatusFlag reduce_scatter(ProcessingRulesPtr rule_ptr) = 0;

        [[nodiscard]] bool cluster_started() const { return _cluster_started; };

    protected:
        virtual void cluster_connect_children(const IdentitySet& child_nodes) = 0;

        virtual void cluster_start() = 0;

        virtual void cluster_end() = 0;

        virtual bool from_sub_cluster(const IdentityPtr& from_id) = 0;

        virtual StatusFlag handle_received_message(const IdentityPtr& from_id, const MetaData& meta,
                                                   Variant& variant);

        virtual StatusFlag handle_disconnect_request(const IdentityPtr& from_id) = 0;

    private:
        StatusFlag handle_message(Handle::MessageEvent& event) final;

    };

#define ClusterFunOverride \
    StatusFlag broadcast(ProcessingRulesPtr rule_ptr) override; \
    \
    StatusFlag scatter(ProcessingRulesPtr rule_ptr) override; \
    \
    StatusFlag reduce(ProcessingRulesPtr rule_ptr) override; \
    \
    StatusFlag all_reduce(ProcessingRulesPtr rule_ptr) override; \
    \
    StatusFlag reduce_scatter(ProcessingRulesPtr rule_ptr) override;

}
