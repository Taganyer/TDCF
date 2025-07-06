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

    TDCF_CHECK_EXPR(child_nodes.find(nullptr) == child_nodes.end())
    TDCF_CHECK_EXPR(child_nodes.find(_handle.self_identity()) == child_nodes.end())

    for (auto& id : child_nodes) {
        _handle.connect(id);
    }
    _handle.create_cluster_data<IdentityList>(child_nodes.begin(), child_nodes.end());
}

bool StarCluster::come_from_children(const IdentityPtr& from_id) {
    if (!_node_agent_started) return true;
    return !from_id->equal_to(*_handle.agent_data<IdentityPtr>());
}

void StarCluster::cluster_start() {
    MetaData meta;
    meta.operation_type = OperationType::AgentCreate;
    meta.stage = Star::start;
    meta.data1[0] = ClusterType::star;
    for (auto& id : _handle.cluster_data<IdentityList>()) {
        StatusFlag flag = _handle.send_message(id, meta, create_node_data());
        TDCF_CHECK_SUCCESS(flag)
    }
    _handle.agent_factory = std::make_unique<StarAgentFactory>();
}

void StarCluster::cluster_end() {
    MetaData meta;
    meta.operation_type = OperationType::Close;
    meta.stage = Star::close;
    StatusFlag flag = StatusFlag::Success;
    for (auto& id : _handle.cluster_data<IdentityList>()) {
        flag = _handle.send_message(id, meta, nullptr);
        TDCF_CHECK_SUCCESS(flag)
    }
    while (!_handle.cluster_data<IdentityList>().empty()) {
        flag = handle_a_loop();
        TDCF_CHECK_SUCCESS(flag)
    }
}

SerializablePtr StarCluster::create_node_data() {
    return std::make_shared<StarAgent>();
}

StatusFlag StarCluster::handle_received_message(const IdentityPtr& from_id, const MetaData& meta,
                                                Variant& variant) {
    auto iter = _handle.find_progress(meta.version);
    TDCF_CHECK_EXPR(_handle.check_progress(iter))
    auto& [m, progress] = *iter;
    return progress->handle_event(meta, variant, _handle);
}

StatusFlag StarCluster::handle_disconnect_request(const IdentityPtr& from_id) {
    assert(!_handle.has_agent_data() || !from_id->equal_to(*_handle.agent_data<IdentityPtr>()));
    assert(!_handle.delayed_message(from_id));
    _handle.disconnect(from_id);
    auto& id_list = _handle.cluster_data<IdentityList>();
    auto iter = std::lower_bound(id_list.begin(), id_list.end(), from_id,
                                 std::less<IdentityPtr>());
    assert(iter != id_list.end());
    assert((*iter)->equal_to(*from_id));
    _handle.cluster_data<IdentityList>().erase(iter);
    return StatusFlag::Success;
}
