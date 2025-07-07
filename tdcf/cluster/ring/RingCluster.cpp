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

using IdentityList = std::vector<IdentityPtr>;

void RingCluster::cluster_connect_children(const IdentitySet& child_nodes) {
    TDCF_CHECK_EXPR(!child_nodes.empty());
    TDCF_CHECK_EXPR(child_nodes.find(nullptr) == child_nodes.end())
    TDCF_CHECK_EXPR(child_nodes.find(_handle.self_identity()) == child_nodes.end())

    _handle.connect(*child_nodes.begin());
    _handle.create_cluster_data<IdentityList>(child_nodes.begin(), child_nodes.end());
}

void RingCluster::cluster_start() {
    auto& list = _handle.cluster_data<IdentityList>();
    MetaData meta;
    meta.operation_type = OperationType::Init;
    meta.serial = list.size();
    for (auto iter = ++list.begin(); iter != list.end(); ++iter) {
        _handle.send_message(*list.begin(), meta, *iter);
        --meta.serial;
    }
    _handle.send_message(*list.begin(), meta, _handle.self_identity());
    _handle.create_cluster_data<RingClusterData>(*list.begin(), nullptr);
    auto receive = _handle.accept();
    _handle.cluster_data<RingClusterData>().receive = receive;
    _handle.agent_factory = std::unique_ptr<RingAgentFactory>();
}

void RingCluster::cluster_end() {
    MetaData meta;
    meta.operation_type = OperationType::Close;
    meta.stage = Ring::close;
    auto& [send, receive] = _handle.cluster_data<RingClusterData>();
    _handle.send_message(send, meta, nullptr);
    while (receive) {
        StatusFlag flag = handle_a_loop();
        TDCF_CHECK_SUCCESS(flag)
    }
}

bool RingCluster::come_from_children(const IdentityPtr& from_id) {
    auto& [send, receive] = _handle.cluster_data<RingClusterData>();
    return send->equal_to(*from_id) || receive->equal_to(*from_id);
}

SerializablePtr RingCluster::create_node_data() {
    return std::make_shared<RingAgent>();
}

StatusFlag RingCluster::handle_disconnect_request(const IdentityPtr& from_id) {
    auto& [send, receive] = _handle.cluster_data<RingClusterData>();
    assert(receive->equal_to(*from_id));
    _handle.disconnect(from_id);
    receive = nullptr;
    return StatusFlag::Success;
}
