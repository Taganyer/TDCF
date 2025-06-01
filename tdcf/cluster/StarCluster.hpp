//
// Created by taganyer on 25-5-22.
//
#pragma once

#include <list>
#include <map>
#include <tdcf/cluster/Cluster.hpp>
#include <tdcf/base/MetaData.hpp>

namespace tdcf {

    class StarCluster : public Cluster {
    public:
        StarCluster(IdentityPtr ip, CommunicatorPtr cp, ProcessorPtr pp,
                    IdentityPtr root_id, unsigned cluster_size);

        ~StarCluster() override;

    private:
        SerializablePtr create_node_data();

    };

}
