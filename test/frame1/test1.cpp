//
// Created by taganyer on 25-7-4.
//
#include <iostream>
#include <tdcf/cluster/StarCluster.hpp>
#include <test/test.hpp>
#include <test/frame1/Communicator1.hpp>
#include <test/frame1/Data1.hpp>
#include <test/frame1/Identity1.hpp>
#include <test/frame1/ProcessingRules1.hpp>
#include <test/frame1/Processor1.hpp>
#include <tinyBackend/Base/Thread.hpp>


using namespace test;

using namespace tdcf;

using namespace std;

static StarCluster create_star_cluster(uint32_t id, CommShare& share, uint32_t root_id = -1) {
    auto self = std::make_shared<Identity1>(id);
    auto comm = std::make_shared<Communicator1>(id, share);
    auto proc = std::make_shared<Processor1>(id);
    IdentityPtr root = root_id == -1 ? nullptr : std::make_shared<Identity1>(root_id);
    return { std::move(self), std::move(comm), std::move(proc), std::move(root) };
}

static Node create_node(uint32_t id, CommShare& share, uint32_t root_id) {
    auto self = std::make_shared<Identity1>(id);
    auto comm = std::make_shared<Communicator1>(id, share);
    auto proc = std::make_shared<Processor1>(id);
    auto root = std::make_shared<Identity1>(root_id);
    return { std::move(self), std::move(comm), std::move(proc), std::move(root) };
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

static void root(uint32_t id, CommShare& share) {
    StarCluster root = create_star_cluster(id, share);
    root.start(2);
    T_DEBUG << __FUNCTION__ << " [" << id << "] started";

    uint32_t tasks_size = 0;
    uint32_t serial = 0;
    StatusFlag flag = StatusFlag::Success;

    creat_task(serial, tasks_size, root, OperationType::Broadcast);
    creat_task(serial, tasks_size, root, OperationType::Scatter);
    creat_task(serial, tasks_size, root, OperationType::Reduce);
    creat_task(serial, tasks_size, root, OperationType::AllReduce);
    creat_task(serial, tasks_size, root, OperationType::ReduceScatter);

    while (flag == StatusFlag::Success && tasks_size > 0) {
        flag = root.handle_a_loop();
    }

    global_logger.flush();
    T_DEBUG << __FUNCTION__ << " [" << id << "] start end: " << status_flag_name(flag);
    cerr << __FUNCTION__ << " [" << id << "] start end: " << status_flag_name(flag) << endl;
    flag = root.end_cluster();
    T_FATAL << __FUNCTION__ << " [" << id << "] end: " << status_flag_name(flag);
    cerr << __FUNCTION__ << " [" << id << "] end: " << status_flag_name(flag) << endl;
}

static void node_root(uint32_t id, CommShare& share, uint32_t root_id) {
    StarCluster node_root = create_star_cluster(id, share, root_id);
    node_root.start(2);
    T_DEBUG << __FUNCTION__ << " " << root_id << "+" << id << " started";

    uint32_t tasks_size = 0;
    uint32_t serial = 0;
    StatusFlag flag = StatusFlag::Success;

    creat_task(serial, tasks_size, node_root, OperationType::Broadcast);
    creat_task(serial, tasks_size, node_root, OperationType::Scatter);
    creat_task(serial, tasks_size, node_root, OperationType::Reduce);
    creat_task(serial, tasks_size, node_root, OperationType::AllReduce);
    creat_task(serial, tasks_size, node_root, OperationType::ReduceScatter);

    bool root_end = false;
    while (flag == StatusFlag::Success && (tasks_size > 0 || !root_end)) {
        flag = node_root.handle_a_loop();
        if (flag == StatusFlag::ClusterOffline) {
            root_end = true;
            flag = StatusFlag::Success;
        }
    }

    T_DEBUG << __FUNCTION__ << " " << root_id << "+" << id << " start end: " << status_flag_name(flag);
    cerr << __FUNCTION__ << " " << root_id << "+" << id << " start end: " << status_flag_name(flag);
    global_logger.flush();
    flag = node_root.end_cluster();
    T_FATAL << __FUNCTION__ << " " << root_id << "+" << id << " end: " << status_flag_name(flag);
    cerr << __FUNCTION__ << " " << root_id << "+" << id << " end: " << status_flag_name(flag) << endl;
    global_logger.flush();
}

static void pure_node(uint32_t id, CommShare& share, uint32_t root_id) {
    Node node1 = create_node(id, share, root_id);
    node1.start(0);
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

    uint32_t root_id = ++serial;
    Base::Thread root_t([root_id, &share] {
        root(root_id, share);
    });

    ++serial;
    Base::Thread node1_t([serial, &share, root_id] {
        pure_node(serial, share, root_id);
    });

    uint32_t root1_id = ++serial;
    Base::Thread root1_t([root1_id, &share, root_id] {
        node_root(root1_id, share, root_id);
    });

    ++serial;
    Base::Thread node11_t([serial, &share, root1_id] {
        pure_node(serial, share, root1_id);
    });

    ++serial;
    Base::Thread node12_t([serial, &share, root1_id] {
        pure_node(serial, share, root1_id);
    });

    root_t.start();
    node1_t.start();
    root1_t.start();
    node11_t.start();
    node12_t.start();

    root_t.join();
    node1_t.join();
    root1_t.join();
    node11_t.join();
    node12_t.join();
}
