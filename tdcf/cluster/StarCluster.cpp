//
// Created by taganyer on 25-5-22.
//

#include <tdcf/cluster/StarCluster.hpp>

using namespace tdcf;


StarCluster::~StarCluster() {
}

StatusFlag StarCluster::handle_a_loop() {
    return StatusFlag::Success;
}
