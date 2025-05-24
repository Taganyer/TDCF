//
// Created by taganyer on 25-5-24.
//

#include <tdcf/cluster/Cluster.hpp>

using namespace tdcf;

#define OperationStructAndFun(struct_t, fun_t) \
struct Cluster::struct_t : ClusterCommand { \
    explicit struct_t(MetaDataPtr mp, ProcessingRulesPtr rp) : \
        ClusterCommand(_##struct_t), data(std::move(mp)), rule(std::move(rp)) {}; \
    MetaDataPtr data; \
    ProcessingRulesPtr rule; \
}; \
void Cluster::fun_t(MetaDataPtr meta_data_ptr, ProcessingRulesPtr rule_ptr) { \
    _command_queue.emplace(std::make_unique<struct_t>(std::move(meta_data_ptr), std::move(rule_ptr))); \
};

OperationStructAndFun(Broadcast, broadcast)

OperationStructAndFun(Scatter, scatter)

OperationStructAndFun(Reduce, reduce)

OperationStructAndFun(AllGather, all_gather)

OperationStructAndFun(AllReduce, all_reduce)

OperationStructAndFun(ReduceScatter, reduce_scatter)

OperationStructAndFun(AllToAll, all_to_all)
