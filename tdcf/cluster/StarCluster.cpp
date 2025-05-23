//
// Created by taganyer on 25-5-22.
//

#include "StarCluster.hpp"

using namespace tdcf;

enum {
    _SetRoot,
    _RootChange,
    _JoinInCluster,
    _Broadcast,
    _Scatter,
    _Reduce,
    _AllGather,
    _AllReduce,
    _ReduceScatter,
    _AllToAll,
};

struct StarCluster::SetRoot : detail::ClusterCommand {
    explicit SetRoot(Iterator iter) : ClusterCommand(_SetRoot), root(iter) {};
    Iterator root;
};

struct StarCluster::RootChange : detail::ClusterCommand {
    explicit RootChange(Iterator iter) : ClusterCommand(_RootChange), root(iter) {};
    Iterator root;
};

struct StarCluster::JoinInCluster : detail::ClusterCommand {
    explicit JoinInCluster(IdentityPtr id) : ClusterCommand(_JoinInCluster), id(std::move(id)) {};
    IdentityPtr id;
};

#define OperationStructAndFun(struct_t, fun_t) \
    struct StarCluster::struct_t : detail::ClusterCommand { \
        explicit struct_t(MetaDataPtr mp, ProcessingRulesPtr rp) : \
            ClusterCommand(_##struct_t), data(std::move(mp)), rule(std::move(rp)) {}; \
        MetaDataPtr data; \
        ProcessingRulesPtr rule; \
    }; \
    void StarCluster::fun_t(MetaDataPtr meta_data_ptr,  ProcessingRulesPtr rule_ptr) { \
        _command_queue.emplace(std::make_unique<struct_t>(std::move(meta_data_ptr), std::move(rule_ptr))); \
    };

OperationStructAndFun(Broadcast, broadcast)
OperationStructAndFun(Scatter, scatter)
OperationStructAndFun(Reduce, reduce)
OperationStructAndFun(AllGather, all_gather)
OperationStructAndFun(AllReduce, all_reduce)
OperationStructAndFun(ReduceScatter, reduce_scatter)
OperationStructAndFun(AllToAll, all_to_all)


StarCluster::~StarCluster() {
}

void StarCluster::join_in_cluster(const IdentityPtr& cluster_id) {
    _command_queue.emplace(std::make_unique<JoinInCluster>(cluster_id));
}

StatusFlag StarCluster::set_root_node(const IdentityPtr& id) {
    auto iter = _nodes.find(id);
    if (iter == _nodes.end()) return StatusFlag::TargetNotFound;
    if (_root != _nodes.end()) {
        _command_queue.emplace(std::make_unique<RootChange>(iter));
        return StatusFlag::Transition;
    }
    _command_queue.emplace(std::make_unique<SetRoot>(iter));
    return StatusFlag::Success;
}

StatusFlag StarCluster::handle_a_loop() {
    return StatusFlag::Success;
}