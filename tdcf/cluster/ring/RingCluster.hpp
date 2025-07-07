//
// Created by taganyer on 25-7-6.
//
#pragma once

#include <tdcf/cluster/Cluster.hpp>

namespace tdcf {

    class RingCluster : public Cluster {
    public:
        RingCluster(IdentityPtr ip, CommunicatorPtr cp, ProcessorPtr pp) :
            Cluster(std::move(ip), std::move(cp), std::move(pp)) {};

        ClusterFunOverride

        struct RingClusterData {
            IdentityPtr send;
            IdentityPtr receive;

            RingClusterData(IdentityPtr send, IdentityPtr receive) :
                send(std::move(send)), receive(std::move(receive)) {};
        };

    private:
        ProcessorAgentFactoryInherit(RingAgentFactory)

        void cluster_connect_children(const IdentitySet& child_nodes) override;

        void cluster_start() override;

        void cluster_end() override;

        bool come_from_children(const IdentityPtr& from_id) override;

        static SerializablePtr create_node_data();

        StatusFlag handle_disconnect_request(const IdentityPtr& from_id) override;

    };

}
