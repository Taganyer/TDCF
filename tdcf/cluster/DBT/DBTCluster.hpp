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

            IdentityPtr t1_parent;

            IdentityPtr t2_parent;

            DBTClusterData(IdentityPtr red_child, IdentityPtr black_child,
                           IdentityPtr t1_parent, IdentityPtr t2_parent) :
                red_child(std::move(red_child)), black_child(std::move(black_child)),
                t1_parent(std::move(t1_parent)), t2_parent(std::move(t2_parent)) {};

        };

        ProcessorAgentFactoryInherit(DBTAgentFactory)

        void cluster_connect_children(const IdentitySet& child_nodes) override;

        void cluster_start() override;

        void send_message_to_child(const std::vector<IdentityPtr>& node_list,
                                   const dbt::DBTInfo& dbt_info);

        void link(IdentityPtr t1_left, uint32_t t1_left_color,
                  IdentityPtr t1_right, IdentityPtr t2_parent);

        void cluster_end() override;

        bool from_sub_cluster(const IdentityPtr& from_id) override;

        static SerializablePtr create_node_data();

        StatusFlag handle_disconnect_request(const IdentityPtr& from_id) override;

    };

} // tdcf
