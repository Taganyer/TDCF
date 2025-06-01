//
// Created by taganyer on 25-5-22.
//

#include <utility>
#include <tdcf/cluster/StarCluster.hpp>

using namespace tdcf;


StarCluster::StarCluster(IdentityPtr ip, CommunicatorPtr cp,
                         ProcessorPtr pp, IdentityPtr root_id,
                         unsigned cluster_size) :
    Cluster(std::move(ip), std::move(cp), std::move(pp), std::move(root_id)) {

}

StarCluster::~StarCluster() {
}
