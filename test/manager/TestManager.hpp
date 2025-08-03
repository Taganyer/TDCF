//
// Created by taganyer on 25-8-3.
//
#pragma once

#include <Base/Thread.hpp>
#include <tdcf/cluster/Cluster.hpp>
#include <test/manager/ComponentCreator.hpp>

namespace test {

    class ClusterInfo;

    using ClusterInfoPtr = std::unique_ptr<ClusterInfo>;

    class ClusterInfo {
    public:
        enum Type {
            Star = 1,
            Ring = 2,
            DBT = 3
        };

        explicit ClusterInfo(Type type, uint32_t pure_nodes, uint32_t o1,
                             uint32_t o2, uint32_t o3, uint32_t o4, uint32_t o5);

        void add_pure_node(uint32_t size);

        void add_sub_cluster(ClusterInfoPtr info);

        void add_operation(tdcf::OperationType type, uint32_t size);

        static ClusterInfoPtr get(Type type, uint32_t pure_nodes, uint32_t o1,
                                  uint32_t o2, uint32_t o3, uint32_t o4, uint32_t o5);

    private:
        Type _type;

        uint32_t _pure_nodes = 0;

        uint32_t _operation[5] {};

        std::vector<ClusterInfoPtr> _sub_clusters;

        friend class TestManager;

    };

    class TestManager {
    public:
        explicit TestManager(ComponentCreatorPtr creator) : _creator(std::move(creator)) {};

        void run(ClusterInfo& info);

    private:
        void create_sub_cluster(uint32_t root_id, uint32_t& serial, ClusterInfo& info);

        using ClusterPtr = std::shared_ptr<tdcf::Cluster>;

        ClusterPtr create_cluster(uint32_t type, uint32_t id) const;

        tdcf::Node create_node(uint32_t id) const;

        tdcf::StatusFlag creat_task(uint32_t& serial, uint32_t& tasks_size,
                                    tdcf::Cluster& root, tdcf::OperationType type) const;

        void root(uint32_t type, uint32_t id, std::vector<uint32_t>& cluster) const;

        void node_root(uint32_t type, uint32_t id, uint32_t root_id,
                       std::vector<uint32_t>& cluster) const;

        void pure_node(uint32_t id, uint32_t root_id) const;

        ComponentCreatorPtr _creator;

        std::vector<Base::Thread> _threads;

    };

}
