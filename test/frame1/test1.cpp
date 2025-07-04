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

static StarCluster create_star_cluster(uint32_t id, CommShare &share, uint32_t root_id = -1) {
    auto self = std::make_shared<Identity1>(id);
    auto comm = std::make_shared<Communicator1>(id, share);
    auto proc = std::make_shared<Processor1>(id);
    IdentityPtr root = root_id == -1 ? nullptr : std::make_shared<Identity1>(root_id);
    return { std::move(self), std::move(comm), std::move(proc), std::move(root) };
}

static Node create_node(uint32_t id, CommShare &share, uint32_t root_id) {
    auto self = std::make_shared<Identity1>(id);
    auto comm = std::make_shared<Communicator1>(id, share);
    auto proc = std::make_shared<Processor1>(id);
    auto root = std::make_shared<Identity1>(root_id);
    return { std::move(self), std::move(comm), std::move(proc), std::move(root) };
}

static void root(uint32_t id, CommShare &share) {
    T_DEBUG << __PRETTY_FUNCTION__ << " started";
    StarCluster root = create_star_cluster(id, share);
    root.start(2);
    T_DEBUG << __FUNCTION__ << " started";
    uint32_t serial = 0;
    StatusFlag flag = root.broadcast(std::make_shared<ProcessingRules1>(++serial, OperationType::Broadcast));
    T_DEBUG << "operation broadcast: " << status_flag_name(flag);
    while (flag == StatusFlag::Success) {
        flag = root.handle_a_loop();
        // T_DEBUG << __FUNCTION__ << " handle a loop: " << status_flag_name(flag);
    }

    global_logger.flush();
    T_DEBUG << __FUNCTION__ << " start end: " << status_flag_name(flag);
    cerr << __FUNCTION__ << " start end: " << status_flag_name(flag);
    flag = root.end_cluster();
    T_DEBUG << __FUNCTION__ << " end: " << status_flag_name(flag);
    cerr << __FUNCTION__ <<" end: " << status_flag_name(flag) << endl;
}

static void node1(uint32_t id, CommShare &share, uint32_t root_id) {
    T_DEBUG << __PRETTY_FUNCTION__ << " started";
    Node node1 = create_node(id, share, root_id);
    node1.start(0);
    T_DEBUG << __FUNCTION__ << " started";

    StatusFlag flag = StatusFlag::Success;
    while (flag == StatusFlag::Success) {
        flag = node1.handle_a_loop();
        // T_DEBUG << __FUNCTION__ << " handle a loop: " << status_flag_name(flag);
    }

    global_logger.flush();
    T_FATAL << __FUNCTION__ << " end: " << status_flag_name(flag);
    cerr << __FUNCTION__ <<" end: " << status_flag_name(flag) << endl;
}

static void root1(uint32_t id, CommShare &share, uint32_t root_id) {
    T_DEBUG << __PRETTY_FUNCTION__ << " started";
    StarCluster root1 = create_star_cluster(id, share, root_id);
    root1.start(1);
    T_DEBUG << __FUNCTION__ << " started";

    StatusFlag flag = StatusFlag::Success;
    while (flag == StatusFlag::Success) {
        flag = root1.handle_a_loop();
        // T_DEBUG << __FUNCTION__ << " handle a loop: " << status_flag_name(flag);
    }

    global_logger.flush();
    T_DEBUG << __FUNCTION__ << " start end: " << status_flag_name(flag);
    cerr << __FUNCTION__ << " start end: " << status_flag_name(flag);
    flag = root1.end_cluster();
    T_DEBUG << __FUNCTION__ << " end: " << status_flag_name(flag);
    cerr << __FUNCTION__ <<" end: " << status_flag_name(flag) << endl;
}

static void node11(uint32_t id, CommShare &share, uint32_t root_id) {
    T_DEBUG << __PRETTY_FUNCTION__ << " started";
    Node node11 = create_node(id, share, root_id);
    node11.start(0);
    T_DEBUG << __FUNCTION__ << " started";

    StatusFlag flag = StatusFlag::Success;
    while (flag == StatusFlag::Success) {
        flag = node11.handle_a_loop();
        // T_DEBUG << __FUNCTION__ << " handle a loop: " << status_flag_name(flag);
    }

    global_logger.flush();
    T_DEBUG << __FUNCTION__ << " end: " << status_flag_name(flag);
    cerr << __FUNCTION__ <<" end: " << status_flag_name(flag) << endl;
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
        node1(serial, share, root_id);
    });

    uint32_t root1_id = ++serial;
    Base::Thread root1_t([root1_id, &share, root_id] {
        root1(root1_id, share, root_id);
    });

    ++serial;
    Base::Thread node11_t([serial, &share, root1_id] {
        node11(serial, share, root1_id);
    });

    root_t.start();
    node1_t.start();
    root1_t.start();
    node11_t.start();

    root_t.join();
    node1_t.join();
    root1_t.join();
    node11_t.join();
}