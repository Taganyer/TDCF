//
// Created by taganyer on 25-8-3.
//
#include <iostream>
#include <tdcf/base/Errors.hpp>
#include <tdcf/cluster/DBT/DBTCluster.hpp>
#include <tdcf/cluster/ring/RingCluster.hpp>
#include <tdcf/cluster/star/StarCluster.hpp>
#include <test/manager/TestManager.hpp>
#include <test/Log.hpp>

using namespace test;

using namespace tdcf;

using namespace std;


ClusterInfo::ClusterInfo(Type type, uint32_t pure_nodes, uint32_t o1,
                         uint32_t o2, uint32_t o3, uint32_t o4, uint32_t o5) :
    _type(type), _pure_nodes(pure_nodes) {
    _operation[0] = o1;
    _operation[1] = o2;
    _operation[2] = o3;
    _operation[3] = o4;
    _operation[4] = o5;
}

void ClusterInfo::add_pure_node(uint32_t size) {
    _pure_nodes += size;
}

void ClusterInfo::add_sub_cluster(ClusterInfoPtr info) {
    _sub_clusters.push_back(std::move(info));
}

void ClusterInfo::add_operation(OperationType type, uint32_t size) {
    int i = (int) type;
    if (i < (int) OperationType::Broadcast || i > (int) OperationType::ReduceScatter)
        return;
    i -= (int) OperationType::Broadcast;
    _operation[i] += size;
}

ClusterInfoPtr ClusterInfo::get(Type type, uint32_t pure_nodes, uint32_t o1,
                                uint32_t o2, uint32_t o3, uint32_t o4, uint32_t o5) {
    return std::make_unique<ClusterInfo>(type, pure_nodes, o1, o2, o3, o4, o5);
}

void TestManager::run(ClusterInfo& info) {
    std::vector<uint32_t> ids;

    uint32_t serial = 1;
    for (uint32_t i = 0; i < info._pure_nodes; ++i) {
        ids.push_back(serial);
        _threads.emplace_back([this, serial] {
            this->pure_node(serial, 0);
        });
        ++serial;
    }
    for (auto& sub : info._sub_clusters) {
        ids.push_back(serial);
        create_sub_cluster(0, serial, *sub);
    }

    _threads.emplace_back([this, type = info._type, &ids, ops = info._operation]
    () mutable {
            this->root(type, 0, ids, ops);
        });

    for (auto& thread : _threads) {
        thread.start();
    }
    for (auto& thread : _threads) {
        thread.join();
    }
}

void TestManager::create_sub_cluster(uint32_t root_id, uint32_t& serial, ClusterInfo& info) {
    uint32_t id = serial++;

    auto type = info._type;
    auto ids = new std::vector<uint32_t>();
    auto ops = new std::array<uint32_t, 5>(info._operation);

    for (uint32_t i = 0; i < info._pure_nodes; ++i) {
        ids->push_back(serial);
        _threads.emplace_back([this, serial, id] {
            this->pure_node(serial, id);
        });
        ++serial;
    }
    for (auto& sub : info._sub_clusters) {
        ids->push_back(serial);
        create_sub_cluster(id, serial, *sub);
    }

    _threads.emplace_back([this, type, id, root_id, ids, ops] {
        this->node_root(type, id, root_id, *ids, *ops);
        delete ids;
        delete ops;
    });
}

TestManager::ClusterPtr TestManager::create_cluster(uint32_t type, uint32_t id) const {
    auto self = _creator->getIdentityPtr(id);
    auto comm = _creator->getCommunicator(id);
    auto proc = _creator->getProcessPtr(id);
    if (type == ClusterType::star) {
        return std::make_shared<StarCluster>(std::move(self), std::move(comm), std::move(proc));
    }
    if (type == ClusterType::ring) {
        return std::make_shared<RingCluster>(std::move(self), std::move(comm), std::move(proc));
    }
    if (type == ClusterType::dbt) {
        return std::make_shared<DBTCluster>(std::move(self), std::move(comm), std::move(proc));
    }
    TDCF_RAISE_ERROR(error cluster type)
}

Node TestManager::create_node(uint32_t id) const {
    auto self = _creator->getIdentityPtr(id);
    auto comm = _creator->getCommunicator(id);
    auto proc = _creator->getProcessPtr(id);
    return { std::move(self), std::move(comm), std::move(proc) };
}

void TestManager::creat_task(uint32_t& serial, uint32_t& tasks_size,
                             Cluster& root, std::array<uint32_t, 5>& ops) const {
    while (true) {
        bool con = false;
        for (int i = 0; i < 5; ++i) {
            if (ops[i] != 0) {
                --ops[i];
                con = true;

                OperationType type = static_cast<OperationType>(i + (int) OperationType::Broadcast);
                auto rule = _creator->getProcessingRulesPtr(++serial, type,
                                                            [&tasks_size] { --tasks_size; });
                ++tasks_size;
                StatusFlag flag = StatusFlag::EventEnd;
                switch (type) {
                    case OperationType::Broadcast:
                        flag = root.broadcast(rule);
                        break;
                    case OperationType::Scatter:
                        flag = root.scatter(rule);
                        break;
                    case OperationType::Reduce:
                        flag = root.reduce(rule);
                        break;
                    case OperationType::AllReduce:
                        flag = root.all_reduce(rule);
                        break;
                    case OperationType::ReduceScatter:
                        flag = root.reduce_scatter(rule);
                        break;
                    default:
                        break;
                }
                TDCF_CHECK_SUCCESS(flag)
                T_DEBUG << "create operation " << operation_type_name(type) << ": " << status_flag_name(flag);
            }
        }
        if (!con) break;
    }
}

void TestManager::root(uint32_t type, uint32_t id, std::vector<uint32_t>& cluster,
                       std::array<uint32_t, 5>& ops) const {
    ClusterPtr root = create_cluster(type, id);

    Cluster::IdentitySet set;
    for (auto i : cluster) {
        set.emplace(_creator->getIdentityPtr(i));
    }

    root->start_cluster(set, false);

    T_DEBUG << __FUNCTION__ << " [" << id << "] started";

    uint32_t tasks_size = 0;
    uint32_t serial = 0;
    StatusFlag flag = StatusFlag::Success;

    creat_task(serial, tasks_size, *root, ops);

    while (flag == StatusFlag::Success && tasks_size > 0) {
        flag = root->handle_a_loop();
    }

    T_DEBUG << __FUNCTION__ << " [" << id << "] start end: " << status_flag_name(flag);
    cerr << __FUNCTION__ << " [" << id << "] start end: " << status_flag_name(flag) << endl;
    flag = root->end_cluster();
    T_FATAL << __FUNCTION__ << " [" << id << "] end: " << status_flag_name(flag);
    cerr << __FUNCTION__ << " [" << id << "] end: " << status_flag_name(flag) << endl;
}

void TestManager::node_root(uint32_t type, uint32_t id, uint32_t root_id,
                            std::vector<uint32_t>& cluster,
                            std::array<uint32_t, 5>& ops) const {
    auto node_root = create_cluster(type, id);

    Cluster::IdentitySet set;
    for (auto i : cluster) {
        set.emplace(_creator->getIdentityPtr(i));
    }

    node_root->start_cluster(set, true);
    T_DEBUG << __FUNCTION__ << " " << root_id << "+" << id << " started";

    uint32_t tasks_size = 0;
    uint32_t serial = 0;
    StatusFlag flag = StatusFlag::Success;

    creat_task(serial, tasks_size, *node_root, ops);

    bool root_end = false;
    while (flag == StatusFlag::Success && (tasks_size > 0 || !root_end)) {
        flag = node_root->handle_a_loop();
        if (flag == StatusFlag::ClusterOffline) {
            root_end = true;
            flag = StatusFlag::Success;
        }
    }

    T_DEBUG << __FUNCTION__ << " " << root_id << "+" << id << " start end: " << status_flag_name(flag);
    cerr << __FUNCTION__ << " " << root_id << "+" << id << " start end: " << status_flag_name(flag) << endl;
    flag = node_root->end_cluster();
    T_FATAL << __FUNCTION__ << " " << root_id << "+" << id << " end: " << status_flag_name(flag);
    cerr << __FUNCTION__ << " " << root_id << "+" << id << " end: " << status_flag_name(flag) << endl;
}

auto TestManager::pure_node(uint32_t id, uint32_t root_id) const -> void {
    Node node = create_node(id);
    node.start_node();
    T_DEBUG << __FUNCTION__ << " " << root_id << "-" << id << " started";

    StatusFlag flag = StatusFlag::Success;
    while (flag == StatusFlag::Success) {
        flag = node.handle_a_loop();
    }

    T_FATAL << __FUNCTION__ << " " << root_id << "-" << id << " end: " << status_flag_name(flag);
    cerr << __FUNCTION__ << " " << root_id << "-" << id << " end: " << status_flag_name(flag) << endl;
}
