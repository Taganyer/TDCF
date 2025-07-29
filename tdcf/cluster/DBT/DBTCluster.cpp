//
// Created by taganyer on 25-7-16.
//
#include <tdcf/base/Errors.hpp>
#include <tdcf/base/DBT/DBT.hpp>
#include <tdcf/cluster/DBT/DBTCluster.hpp>
#include <tdcf/node/agents/DBT/DBTAgent.hpp>

using namespace tdcf;


#define DBTClusterFun(fun_name, class_name) \
StatusFlag DBTCluster::fun_name(ProcessingRulesPtr rule_ptr) { \
    StatusFlag flag = class_name::create(std::move(rule_ptr), _handle); \
    if (flag != StatusFlag::Success) return flag; \
    return flag; \
}

#define DBTAgentFactoryFun(type, class_name) \
StatusFlag DBTCluster::DBTAgentFactory::type(const ProcessingRulesPtr& rule, ProgressEventsMI iter, \
    Handle& handle, EventProgressAgent **agent_ptr) { \
    return class_name::create(rule, iter, handle, agent_ptr); \
}

#define DBTFunAll(fun, cluster_class, agent_class) \
    DBTClusterFun(fun, cluster_class) \
    DBTAgentFactoryFun(fun, agent_class)


#define DBTEmpty(fun, cluster_class, agent_class) \
StatusFlag DBTCluster::fun(ProcessingRulesPtr rule_ptr) { \
    return StatusFlag::Success; \
} \
StatusFlag DBTCluster::DBTAgentFactory::fun(const ProcessingRulesPtr& rule, ProgressEventsMI iter, \
    Handle& handle, EventProgressAgent **agent_ptr) { \
    return StatusFlag::Success; \
}

DBTFunAll(broadcast, Broadcast, BroadcastAgent)

DBTFunAll(scatter, Scatter, ScatterAgent)

DBTEmpty(reduce, Reduce, ReduceAgent)

DBTEmpty(all_reduce, AllReduce, AllReduceAgent)

DBTEmpty(reduce_scatter, ReduceScatter, ReduceScatterAgent)

void DBTCluster::cluster_connect_children(const IdentitySet& child_nodes) {
    TDCF_CHECK_EXPR(!child_nodes.empty())
    TDCF_CHECK_EXPR(child_nodes.find(nullptr) == child_nodes.end())
    TDCF_CHECK_EXPR(child_nodes.find(_handle.self_identity()) == child_nodes.end())

    _handle.create_cluster_data<std::pair<IdentitySet, dbt::DBTInfo>>(
        child_nodes, dbt::creat_dbt(child_nodes.size()));
    for (auto& child : child_nodes) {
        _handle.connect(child);
    }
}

void DBTCluster::cluster_start() {
    auto& nodes = _handle.cluster_data<std::pair<IdentitySet, dbt::DBTInfo>>().first;
    auto& dbt_info = _handle.cluster_data<std::pair<IdentitySet, dbt::DBTInfo>>().second;
    auto& [root1, root2, array] = dbt_info;

    MetaData meta;
    meta.operation_type = OperationType::AgentCreate;
    meta.data1[0] = ClusterType::dbt;
    meta.stage = DBT::start;

    std::vector<IdentityPtr> node_list(array.size());

    uint32_t serial = 0;
    for (auto& node : nodes) {
        meta.serial = serial;
        node_list[serial] = node;
        _handle.send_message(node, meta, create_node_data());
        ++serial;
    }

    send_message_to_child(node_list, dbt_info);

    IdentityPtr t1_root = node_list[root1];
    IdentityPtr t2_root = node_list[root2];

    _handle.destroy_cluster_data();

    _handle.connect(t1_root);
    if (!equal_to(t1_root, t2_root)) {
        _handle.connect(t2_root);
    }

    _handle.create_cluster_data<DBTClusterData>(std::move(t1_root), std::move(t2_root), array.size());

}

void DBTCluster::send_message_to_child(const std::vector<IdentityPtr>& node_list,
                                       const dbt::DBTInfo& dbt_info) {
    auto& [root1, root2, array] = dbt_info;

    MetaData meta;
    meta.operation_type = OperationType::Init;

    for (uint32_t i = 0; i < array.size(); ++i) {
        auto& [t1_parent, t1_left, t1_right, t1_color,
            t2_parent, t2_left, t2_right, t2_color] = array[i];

        /// is_leaf_node_in_t1
        meta.data1[0] = t1_left == -1 && t1_right == -1;
        /// is_leaf_node_in_t2
        meta.data1[1] = t2_left == -1 && t2_right == -1;
        meta.serial = array.size();
        _handle.send_message(node_list[i], meta,
                             t1_parent != -1 ? node_list[t1_parent] : _handle.self_identity());

        _handle.send_message(node_list[i], meta,
                             t2_parent != -1 ? node_list[t2_parent] : _handle.self_identity());

        if (t1_left != -1 || t1_right != -1) {
            if (t1_left != -1) {
                meta.serial = t1_left;
                meta.data1[2] = array[t1_left].t1_color;
                _handle.send_message(node_list[i], meta, node_list[t1_left]);
            }
            if (t1_right != -1) {
                meta.serial = t1_right;
                meta.data1[2] = array[t1_right].t1_color;
                _handle.send_message(node_list[i], meta, node_list[t1_right]);
            }
        } else {
            if (t2_left != -1) {
                meta.serial = t2_left;
                meta.data1[2] = array[t2_left].t1_color;
                _handle.send_message(node_list[i], meta, node_list[t2_left]);
            }
            if (t2_right != -1) {
                meta.serial = t2_right;
                meta.data1[2] = array[t2_right].t1_color;
                _handle.send_message(node_list[i], meta, node_list[t2_right]);
            }
        }
    }

    for (uint32_t i = 0; i < array.size(); ++i) {
        _handle.disconnect(node_list[i]);
    }
}

void DBTCluster::cluster_end() {
    auto& [t1, t2, size] = _handle.cluster_data<DBTClusterData>();
    _handle.disconnect(t1);
    if (!t2->equal_to(*t1)) _handle.disconnect(t2);
}

bool DBTCluster::from_sub_cluster(const IdentityPtr& from_id) {
    auto& [t1, t2, size] = _handle.cluster_data<DBTClusterData>();
    return t1->equal_to(*from_id) || t2->equal_to(*from_id);
}

SerializablePtr DBTCluster::create_node_data() {
    return std::make_shared<DBTAgent>();
}

StatusFlag DBTCluster::handle_disconnect_request(const IdentityPtr& from_id) {
    throw std::runtime_error(std::string("unexpect ") + __PRETTY_FUNCTION__);
}
