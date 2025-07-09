//
// Created by taganyer on 25-7-6.
//

#include <tdcf/base/Errors.hpp>
#include <tdcf/cluster/ring/RingCluster.hpp>
#include <tdcf/node/agents/ring/RingAgent.hpp>

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

RingFunAll(broadcast, Broadcast, BroadcastAgent)

RingFunAll(scatter, Scatter, ScatterAgent)

RingFunAll(reduce, Reduce, ReduceAgent)

RingFunAll(all_reduce, AllReduce, AllReduceAgent)

RingFunAll(reduce_scatter, ReduceScatter, ReduceScatterAgent)

using IdentityList = std::vector<IdentityPtr>;

void RingCluster::cluster_connect_children(const IdentitySet& child_nodes) {
    TDCF_CHECK_EXPR(!child_nodes.empty())
    TDCF_CHECK_EXPR(child_nodes.find(nullptr) == child_nodes.end())
    TDCF_CHECK_EXPR(child_nodes.find(_handle.self_identity()) == child_nodes.end())

    _handle.connect(*child_nodes.begin());
    _handle.create_cluster_data<IdentityList>(child_nodes.begin(), child_nodes.end());
}

void RingCluster::cluster_start() {
    auto& list = _handle.cluster_data<IdentityList>();
    MetaData meta;
    meta.operation_type = OperationType::AgentCreate;
    meta.data1[0] = ClusterType::ring;
    meta.stage = Ring::start;
    meta.serial = list.size();
    StatusFlag flag = _handle.send_message(*list.begin(), meta, create_node_data());
    TDCF_CHECK_SUCCESS(flag)

    meta.operation_type = OperationType::Init;
    for (auto iter = ++list.begin(); iter != list.end(); ++iter) {
        flag = _handle.send_message(*list.begin(), meta, *iter);
        TDCF_CHECK_SUCCESS(flag)
        --meta.serial;
    }
    _handle.send_message(*list.begin(), meta, _handle.self_identity());

    if (list.size() > 1) {
        auto receive = _handle.accept();
        _handle.create_cluster_data<RingClusterData>(*list.begin(), receive, list.size());
    } else {
        _handle.create_cluster_data<RingClusterData>(*list.begin(), *list.begin(), list.size());
    }
    _handle.agent_factory = std::make_unique<RingAgentFactory>();
    assert(_handle.agent_factory);
}

void RingCluster::cluster_end() {
    auto& [send, receive, cluster_size] = _handle.cluster_data<RingClusterData>();
    _handle.disconnect(send);
    if (cluster_size == 1) return;
    while (receive) {
        StatusFlag flag = handle_a_loop();
        TDCF_CHECK_SUCCESS(flag)
    }
}

bool RingCluster::come_from_children(const IdentityPtr& from_id) {
    auto& [send, receive, cluster_size] = _handle.cluster_data<RingClusterData>();
    return send->equal_to(*from_id) || receive->equal_to(*from_id);
}

SerializablePtr RingCluster::create_node_data() {
    return std::make_shared<RingAgent>();
}

StatusFlag RingCluster::handle_disconnect_request(const IdentityPtr& from_id) {
    auto& [send, receive, cluster_size] = _handle.cluster_data<RingClusterData>();
    assert(receive->equal_to(*from_id));
    _handle.disconnect(from_id);
    receive = nullptr;
    return StatusFlag::Success;
}
