//
// Created by taganyer on 25-5-22.
//

#include <utility>
#include <tdcf/cluster/StarCluster.hpp>

using namespace tdcf;


StarCluster::StarCluster(IdentityPtr idp, CommunicatorPtr cp, ProcessorPtr pp,
                         InterpreterPtr inp, unsigned cluster_size) :
    Cluster(std::move(idp),std::move(cp), std::move(pp), std::move(inp)) {

}

StarCluster::~StarCluster() {
}
