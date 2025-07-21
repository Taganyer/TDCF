//
// Created by taganyer on 25-7-16.
//
#include <tdcf/base/Errors.hpp>
#include <tdcf/base/DBT/DBT.hpp>
#include <tdcf/cluster/DBT/DBTCluster.hpp>
#include <tdcf/node/agents/DBT/DBTAgent.hpp>

using namespace tdcf;


void DBTCluster::cluster_connect_children(const IdentitySet& child_nodes) {
    TDCF_CHECK_EXPR(!child_nodes.empty())
    TDCF_CHECK_EXPR(child_nodes.find(nullptr) == child_nodes.end())
    TDCF_CHECK_EXPR(child_nodes.find(_handle.self_identity()) == child_nodes.end())

    _handle.create_cluster_data<std::pair<IdentitySet, dbt::DBTInfo>>(
        child_nodes, dbt::creat_dbt(child_nodes.size() + 1));
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
    node_list[root1] = _handle.self_identity();
    uint32_t serial = 0;
    for (auto& node : nodes) {
        if (serial == root1) ++serial;

        meta.serial = serial;
        node_list[serial] = node;
        _handle.send_message(node, meta, create_node_data());
        ++serial;
    }

    send_message_to_child(node_list, dbt_info);

    auto& [t1_parent, t1_left, t1_right, t1_color,
        t2_parent, t2_left, t2_right, t2_color] = array[root1];

    IdentityPtr t1_left_id(t1_left != -1 ? std::move(node_list[t1_left]) : nullptr);
    uint32_t t1_left_color = t1_left != -1 ? array[t1_left].t1_color : 3;
    IdentityPtr t1_right_id(t1_right != -1 ? std::move(node_list[t1_right]) : nullptr);
    assert(t2_parent != -1 && t1_left == -1 && t1_right == -1);
    IdentityPtr t2_parent_id(std::move(node_list[t2_parent]));

    _handle.destroy_cluster_data();

    link(std::move(t1_left_id), t1_left_color, std::move(t1_right_id), std::move(t2_parent_id));
}

void DBTCluster::send_message_to_child(const std::vector<IdentityPtr>& node_list,
                                       const dbt::DBTInfo& dbt_info) {
    auto& [root1, root2, array] = dbt_info;

    MetaData meta;
    meta.operation_type = OperationType::Init;

    for (uint32_t i = 0; i < array.size(); ++i) {
        auto& [t1_parent, t1_left, t1_right, t1_color,
            t2_parent, t2_left, t2_right, t2_color] = array[i];
        if (i == root1) continue;

        assert(t1_parent != -1);
        _handle.send_message(node_list[i], meta, node_list[t1_parent]);
        meta.data1[0] = t1_left != -1 ? array[t1_left].t1_color : 3;
        _handle.send_message(node_list[i], meta, t1_left != -1 ? node_list[t1_left] : nullptr);
        meta.data1[0] = t1_right != -1 ? array[t1_right].t1_color : 3;
        _handle.send_message(node_list[i], meta, t1_right != -1 ? node_list[t1_right] : nullptr);

        _handle.send_message(node_list[i], meta, t2_parent != -1 ? node_list[t2_parent] : nullptr);
        meta.data1[0] = t2_left != -1 ? array[t2_left].t1_color : 3;
        _handle.send_message(node_list[i], meta, t2_left != -1 ? node_list[t2_left] : nullptr);
        meta.data1[0] = t2_right != -1 ? array[t2_right].t1_color : 3;
        _handle.send_message(node_list[i], meta, t2_right != -1 ? node_list[t2_right] : nullptr);
    }

    for (uint32_t i = 0; i < array.size(); ++i) {
        if (i == root1) continue;
        _handle.disconnect(node_list[i]);
    }
}

void DBTCluster::link(IdentityPtr t1_left, uint32_t t1_left_color,
                      IdentityPtr t1_right, IdentityPtr t2_parent) {
    auto id1 = _handle.accept();
    assert(t1_left && id1->equal_to(*t1_left) || t1_right && id1->equal_to(*t1_right));
    auto id2 = t1_left && t1_right ? _handle.accept() : nullptr;
    assert(!id2 || t1_left && id2->equal_to(*t1_left) || t1_right && id2->equal_to(*t1_right));

    if (t2_parent && !t2_parent->equal_to(*id1) && !(id2 && t2_parent->equal_to(*id2))) {
        _handle.connect(t2_parent);
    }

    if (t1_left_color == 0) {
        _handle.create_cluster_data<DBTClusterData>(std::move(t1_left), std::move(t1_right),
                                                    nullptr, std::move(t2_parent));
    } else {
        _handle.create_cluster_data<DBTClusterData>(std::move(t1_right), std::move(t1_left),
                                                    nullptr, std::move(t2_parent));
    }
}

void DBTCluster::cluster_end() {
}

bool DBTCluster::from_sub_cluster(const IdentityPtr& from_id) {
    auto& [red_child, black_child, t1_parent, t2_parent] = _handle.cluster_data<DBTClusterData>();
    return red_child && from_id->equal_to(*red_child) ||
        black_child && from_id->equal_to(*black_child) ||
        t2_parent && from_id->equal_to(*t2_parent);
}

SerializablePtr DBTCluster::create_node_data() {
    return std::make_shared<DBTAgent>();
}

StatusFlag DBTCluster::handle_disconnect_request(const IdentityPtr& from_id) {
}
