//
// Created by taganyer on 25-5-24.
//

#include <tdcf/cluster/Cluster.hpp>

using namespace tdcf;

#define OperationFun(struct_t, fun_t) \
void Cluster::fun_t(ProcessingRulesPtr rule_ptr) { \
    _self_queue.emplace(std::make_unique<struct_t>(std::move(rule_ptr))); \
};

OperationFun(HTCBroadcast, broadcast)

OperationFun(HTCScatter, scatter)

OperationFun(HTCReduce, reduce)

OperationFun(HTCAllGather, all_gather)

OperationFun(HTCAllReduce, all_reduce)

OperationFun(HTCReduceScatter, reduce_scatter)

OperationFun(HTCAllToAll, all_to_all)
