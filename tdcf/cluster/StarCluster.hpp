//
// Created by taganyer on 25-5-22.
//
#pragma once

#include <set>

#include "Cluster.hpp"

namespace tdcf {

    class StarCluster : public Cluster {
    public:
        StarCluster(IdentityPtr ip, CommanderPtr cp, ProcessorPtr pp) :
            Cluster(std::move(ip), std::move(cp), std::move(pp)) {};

        ~StarCluster() override;

        void join_in_cluster(const IdentityPtr& cluster_id) override;

        StatusFlag set_root_node(const IdentityPtr& id) override;

        void broadcast(MetaDataPtr meta_data_ptr,  ProcessingRulesPtr rule_ptr) override;

        void scatter(MetaDataPtr meta_data_ptr,  ProcessingRulesPtr rule_ptr) override;

        void reduce(MetaDataPtr meta_data_ptr,  ProcessingRulesPtr rule_ptr) override;

        void all_gather(MetaDataPtr meta_data_ptr,  ProcessingRulesPtr rule_ptr) override;

        void all_reduce(MetaDataPtr meta_data_ptr,  ProcessingRulesPtr rule_ptr) override;

        void reduce_scatter(MetaDataPtr meta_data_ptr,  ProcessingRulesPtr rule_ptr) override;

        void all_to_all(MetaDataPtr meta_data_ptr,  ProcessingRulesPtr rule_ptr) override;

        StatusFlag handle_a_loop() override;

    private:
        std::set<IdentityPtr, IdentityPtrLess> _nodes;

        using Iterator = std::set<IdentityPtr, IdentityPtrLess>::iterator;

        Iterator _root;

        detail::ClusterCommandQueue _command_queue;

        struct SetRoot;

        struct RootChange;

        struct JoinInCluster;

        struct Broadcast;

        struct Scatter;

        struct Reduce;

        struct AllGather;

        struct AllReduce;

        struct ReduceScatter;

        struct AllToAll;


    };

}
