//
// Created by taganyer on 25-7-16.
//
#pragma once

#include <tdcf/cluster/Cluster.hpp>

namespace tdcf {

    class DBTCluster : public Cluster {
    public:
        DBTCluster(IdentityPtr ip, CommunicatorPtr cp, ProcessorPtr pp) :
            Cluster(std::move(ip), std::move(cp), std::move(pp)) {};

        ClusterFunOverride

    private:
        struct DBTClusterData {
            IdentityPtr red_child;

            IdentityPtr black_child;

            IdentityPtr parent;

            DBTClusterData(IdentityPtr red_child, IdentityPtr black_child, IdentityPtr parent) :
                red_child(std::move(red_child)), black_child(std::move(black_child)), parent(std::move(parent)) {};

        };

        ProcessorAgentFactoryInherit(DBTAgentFactory)

        void cluster_connect_children(const IdentitySet& child_nodes) override;

        void cluster_start() override;

        void cluster_end() override;

        bool come_from_children(const IdentityPtr& from_id) override;

        static SerializablePtr create_node_data();

        StatusFlag handle_disconnect_request(const IdentityPtr& from_id) override;

    };

} // tdcf
