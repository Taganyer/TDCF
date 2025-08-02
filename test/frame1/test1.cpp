//
// Created by taganyer on 25-7-4.
//
#include <iostream>
#include <tdcf/base/Errors.hpp>
#include <tdcf/cluster/ring/RingCluster.hpp>
#include <tdcf/cluster/star/StarCluster.hpp>
#include <test/test.hpp>
#include <test/frame1/Communicator1.hpp>
#include <test/frame1/Data1.hpp>
#include <test/frame1/Identity1.hpp>
#include <test/frame1/ProcessingRules1.hpp>
#include <test/frame1/Processor1.hpp>
#include <tinyBackend/Base/Thread.hpp>

#include "tdcf/cluster/DBT/DBTCluster.hpp"


using namespace test;

using namespace tdcf;

using namespace std;

using ClusterPtr = std::shared_ptr<Cluster>;

static ClusterPtr create_cluster(uint32_t type, uint32_t id, CommShare& share) {
    auto self = std::make_shared<Identity1>(id);
    auto comm = std::make_shared<Communicator1>(id, share);
    auto proc = std::make_shared<Processor1>(id);
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

static Node create_node(uint32_t id, CommShare& share, uint32_t root_id) {
    auto self = std::make_shared<Identity1>(id);
    auto comm = std::make_shared<Communicator1>(id, share);
    auto proc = std::make_shared<Processor1>(id);
    auto root = std::make_shared<Identity1>(root_id);
    return { std::move(self), std::move(comm), std::move(proc) };
}

static StatusFlag creat_task(uint32_t& serial, uint32_t& tasks_size,
                             Cluster& root, OperationType type) {
    std::shared_ptr<ProcessingRules1> rule = std::make_shared<ProcessingRules1>(++serial, type);
    rule->set_callback([&tasks_size] { --tasks_size; });
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
            assert(false);
    }
    T_DEBUG << "operation " << operation_type_name(type) << ": " << status_flag_name(flag);
    return flag;
}

static void root(uint32_t type, uint32_t id,
                 CommShare& share, Cluster::IdentitySet& cluster) {
    ClusterPtr root = create_cluster(type, id, share);
    root->start_cluster(cluster, false);
    T_DEBUG << __FUNCTION__ << " [" << id << "] started";

    uint32_t tasks_size = 0;
    uint32_t serial = 0;
    StatusFlag flag = StatusFlag::Success;

    creat_task(serial, tasks_size, *root, OperationType::Broadcast);
    creat_task(serial, tasks_size, *root, OperationType::Scatter);
    creat_task(serial, tasks_size, *root, OperationType::Reduce);
    creat_task(serial, tasks_size, *root, OperationType::AllReduce);
    creat_task(serial, tasks_size, *root, OperationType::ReduceScatter);

    while (flag == StatusFlag::Success && tasks_size > 0) {
        flag = root->handle_a_loop();
    }

    global_logger.flush();
    T_DEBUG << __FUNCTION__ << " [" << id << "] start end: " << status_flag_name(flag);
    cerr << __FUNCTION__ << " [" << id << "] start end: " << status_flag_name(flag) << endl;
    flag = root->end_cluster();
    T_FATAL << __FUNCTION__ << " [" << id << "] end: " << status_flag_name(flag);
    cerr << __FUNCTION__ << " [" << id << "] end: " << status_flag_name(flag) << endl;
    // sleep(2);
}

static void node_root(uint32_t type, uint32_t id, uint32_t root_id,
                      CommShare& share, Cluster::IdentitySet& cluster) {
    auto node_root = create_cluster(type, id, share);
    node_root->start_cluster(cluster, true);
    T_DEBUG << __FUNCTION__ << " " << root_id << "+" << id << " started";

    uint32_t tasks_size = 0;
    uint32_t serial = 0;
    StatusFlag flag = StatusFlag::Success;

    creat_task(serial, tasks_size, *node_root, OperationType::Broadcast);
    creat_task(serial, tasks_size, *node_root, OperationType::Scatter);
    creat_task(serial, tasks_size, *node_root, OperationType::Reduce);
    creat_task(serial, tasks_size, *node_root, OperationType::AllReduce);
    creat_task(serial, tasks_size, *node_root, OperationType::ReduceScatter);

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
    global_logger.flush();
    flag = node_root->end_cluster();
    T_FATAL << __FUNCTION__ << " " << root_id << "+" << id << " end: " << status_flag_name(flag);
    cerr << __FUNCTION__ << " " << root_id << "+" << id << " end: " << status_flag_name(flag) << endl;
    global_logger.flush();
}

static void pure_node(uint32_t id, CommShare& share, uint32_t root_id) {
    Node node1 = create_node(id, share, root_id);
    node1.start_node();
    T_DEBUG << __FUNCTION__ << " " << root_id << "-" << id << " started";

    StatusFlag flag = StatusFlag::Success;
    while (flag == StatusFlag::Success) {
        flag = node1.handle_a_loop();
    }

    T_FATAL << __FUNCTION__ << " " << root_id << "-" << id << " end: " << status_flag_name(flag);
    cerr << __FUNCTION__ << " " << root_id << "-" << id << " end: " << status_flag_name(flag) << endl;
    global_logger.flush();
}


void test::correctness_test() {
    uint32_t serial = 0;
    CommShare share;

    Cluster::IdentitySet cluster1, cluster2;

    uint32_t root_id = ++serial;
    // uint32_t node1_id = ++serial;
    // cluster1.insert(std::make_shared<Identity1>(node1_id));
    // uint32_t node2_id = ++serial;
    // cluster1.insert(std::make_shared<Identity1>(node2_id));

    uint32_t root1_id = ++serial;
    cluster1.insert(std::make_shared<Identity1>(root1_id));

    uint32_t node11_id = ++serial;
    cluster2.insert(std::make_shared<Identity1>(node11_id));
    uint32_t node12_id = ++serial;
    cluster2.insert(std::make_shared<Identity1>(node12_id));

    Base::Thread root_t([root_id, &share, &cluster1] {
        root(ClusterType::dbt, root_id, share, cluster1);
    });

    // Base::Thread node1_t([node1_id, &share, root_id] {
    //     pure_node(node1_id, share, root_id);
    // });
    //
    // Base::Thread node2_t([node2_id, &share, root_id] {
    //     pure_node(node2_id, share, root_id);
    // });

    Base::Thread root1_t([root1_id, root_id, &share, &cluster2] {
        node_root(ClusterType::ring, root1_id, root_id, share, cluster2);
    });

    Base::Thread node11_t([node11_id, &share, root1_id] {
        pure_node(node11_id, share, root1_id);
    });

    Base::Thread node12_t([node12_id, &share, root1_id] {
        pure_node(node12_id, share, root1_id);
    });

    root_t.start();
    // node1_t.start();
    // node2_t.start();
    root1_t.start();
    node11_t.start();
    node12_t.start();

    root_t.join();
    // node1_t.join();
    // node2_t.join();
    root1_t.join();
    node11_t.join();
    node12_t.join();
}
