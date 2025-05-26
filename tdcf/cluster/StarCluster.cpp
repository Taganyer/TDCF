//
// Created by taganyer on 25-5-22.
//

#include <tdcf/cluster/StarCluster.hpp>
#include <utility>

using namespace tdcf;


StarCluster::StarCluster(IdentityPtr idp, TransmitterPtr tp, CommanderPtr cp, ProcessorPtr pp,
                         InterpreterPtr inp, unsigned cluster_size) :
    Cluster(std::move(idp), std::move(tp), std::move(cp), std::move(pp), std::move(inp)) {

}

StarCluster::~StarCluster() {
}

StatusFlag StarCluster::handle_a_loop() {
    return StatusFlag::Success;
}
