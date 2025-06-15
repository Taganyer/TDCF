//
// Created by taganyer on 25-5-24.
//

#include <tdcf/base/Errors.hpp>
#include <tdcf/cluster/Cluster.hpp>

using namespace tdcf;

#define OperationFun(struct_t, fun_t) \
void Cluster::fun_t(ProcessingRulesPtr rule_ptr) { \
    auto [iter, success] = _info.progress_events.emplace(_meta, std::make_unique<struct_t>(std::move(rule_ptr))); \
    assert(success); \
    ++_meta; \
};

OperationFun(HTCBroadcast, broadcast)

OperationFun(HTCScatter, scatter)

OperationFun(HTCReduce, reduce)

OperationFun(HTCAllGather, all_gather)

OperationFun(HTCAllReduce, all_reduce)

OperationFun(HTCReduceScatter, reduce_scatter)

OperationFun(HTCAllToAll, all_to_all)

StatusFlag Cluster::active_events() {
    return Node::active_events();
}

StatusFlag Cluster::analysis_messages() {
    return Node::analysis_messages();
}

StatusFlag Cluster::handle_messages() {
    return Node::handle_messages();
}
