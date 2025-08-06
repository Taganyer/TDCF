//
// Created by taganyer on 25-5-22.
//

#include <algorithm>
#include <tdcf/base/Errors.hpp>
#include <tdcf/cluster/star/StarCluster.hpp>
#include <tdcf/node/agents/star/StarAgent.hpp>

using namespace tdcf;


#define StarClusterFun(fun_name, class_name) \
StatusFlag StarCluster::fun_name(ProcessingRulesPtr rule_ptr) { \
    StatusFlag flag = class_name::create(std::move(rule_ptr), _handle); \
    if (flag != StatusFlag::Success) return flag; \
    return flag; \
}

#define StarAgentFactoryFun(type, class_name) \
StatusFlag StarCluster::StarAgentFactory::type(const ProcessingRulesPtr& rule, ProgressEventsMI iter, \
    Handle& handle, EventProgressAgent **agent_ptr) { \
    return class_name::create(rule, iter, handle, agent_ptr); \
}

#define StarFunAll(fun, cluster_class, agent_class) \
    StarClusterFun(fun, cluster_class) \
    StarAgentFactoryFun(fun, agent_class)


StarFunAll(broadcast, Broadcast, BroadcastAgent)

StarFunAll(scatter, Scatter, ScatterAgent)

StarFunAll(reduce, Reduce, ReduceAgent)

StarFunAll(all_reduce, AllReduce, AllReduceAgent)

StarFunAll(reduce_scatter, ReduceScatter, ReduceScatterAgent)

void StarCluster::cluster_connect_children(const IdentitySet& child_nodes) {
    TDCF_CHECK_EXPR(!child_nodes.empty());
    TDCF_CHECK_EXPR(child_nodes.find(nullptr) == child_nodes.end())
    TDCF_CHECK_EXPR(child_nodes.find(_handle.self_identity()) == child_nodes.end())

    for (auto& id : child_nodes) {
        _handle.connect(id);
    }
    _handle.create_cluster_data<IdentityList>(child_nodes.begin(), child_nodes.end());
}

void StarCluster::cluster_start() {
    MetaData meta;
    meta.operation_type = OperationType::AgentCreate;
    meta.stage = Star::start;
    meta.data1[0] = ClusterType::star;
    for (auto& id : _handle.cluster_data<IdentityList>()) {
        StatusFlag flag = _handle.send_message(id, meta, nullptr);
        TDCF_CHECK_SUCCESS(flag)
    }
    _handle.agent_factory = std::make_unique<StarAgentFactory>();

    waiting_respond();
}

void StarCluster::waiting_respond() {
    uint32_t size = _handle.cluster_data<IdentityList>().size();
    while (size > 0) {
        Handle::MessageEvent message;
        _handle.waiting_for_message(message);
        assert(message.type == CommunicatorEvent::ReceivedMessage);
        assert(message.meta.stage == Star::respond);
        --size;
    }
}

void StarCluster::cluster_end() {
    for (auto& id : _handle.cluster_data<IdentityList>()) {
        _handle.disconnect(id);
    }
}

bool StarCluster::from_sub_cluster(const IdentityPtr& from_id) {
    auto& id_list = _handle.cluster_data<IdentityList>();
    auto find = std::binary_search(id_list.begin(), id_list.end(), from_id,
                                   IdentityPtrLess());
    return find;
}

StatusFlag StarCluster::handle_disconnect_request(const IdentityPtr& from_id) {
    TDCF_RAISE_ERROR(unreachable fun)
}
