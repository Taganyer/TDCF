//
// Created by taganyer on 25-7-6.
//
#include <tdcf/cluster/ring/RingCluster.hpp>

using namespace tdcf;


#define RingClusterFun(fun_name, class_name) \
StatusFlag RingCluster::fun_name(ProcessingRulesPtr rule_ptr) { \
    StatusFlag flag = class_name::create(std::move(rule_ptr), _handle); \
    if (flag != StatusFlag::Success) return flag; \
    return flag; \
}

#define RingAgentFactoryFun(type, class_name) \
StatusFlag RingCluster::RingAgentFactory::type(const ProcessingRulesPtr& rule, ProgressEventsMI iter, \
    Handle& handle, EventProgressAgent **agent_ptr) { \
    return class_name::create(rule, iter, handle, agent_ptr); \
}

#define RingFunAll(fun, cluster_class, agent_class) \
    RingClusterFun(fun, cluster_class) \
    RingAgentFactoryFun(fun, agent_class)


void RingCluster::cluster_connect_children(const IdentitySet& child_nodes) {
}

void RingCluster::cluster_start() {
}

void RingCluster::cluster_end() {
}

SerializablePtr RingCluster::create_node_data() {
}

StatusFlag RingCluster::handle_received_message(const IdentityPtr& from_id, const MetaData& meta, Variant& variant) {
}

StatusFlag RingCluster::handle_disconnect_request(const IdentityPtr& from_id) {
}
