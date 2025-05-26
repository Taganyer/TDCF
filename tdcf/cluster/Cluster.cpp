//
// Created by taganyer on 25-5-24.
//

#include <tdcf/cluster/Cluster.hpp>

using namespace tdcf;

#define OperationFun(struct_t, fun_t) \
void Cluster::fun_t(ProcessingRulesPtr rule_ptr) { \
    _self_queue.emplace(std::make_unique<struct_t>(std::move(rule_ptr))); \
};

struct Cluster::ClusterBroadcast : ClusterEvent {
    explicit ClusterBroadcast(ProcessingRulesPtr rp) :
        ClusterEvent(EventType::HTCBroadcast, std::move(rp)) {};

    DataPtr data_ptr;
    bool got_data = false;
    unsigned data_send = 0;
};

struct Cluster::ClusterScatter : ClusterEvent {
    explicit ClusterScatter(ProcessingRulesPtr rp) :
        ClusterEvent(EventType::HTCScatter, std::move(rp)) {};

    DataPtr data_ptr;
    Processor::DataSet data_set;
    bool got_data = false, got_dataset = false;
    unsigned data_send = 0;
};

struct Cluster::ClusterReduce : ClusterEvent {
    explicit ClusterReduce(ProcessingRulesPtr rp) :
        ClusterEvent(EventType::HTCReduce, std::move(rp)) {};

    Processor::DataSet data_set;
    DataPtr data_ptr;
    unsigned data_sent = 0, data_received = 0;
};

struct Cluster::ClusterAllGather : ClusterEvent {
    explicit ClusterAllGather(ProcessingRulesPtr rp) :
        ClusterEvent(EventType::HTCAllGather, std::move(rp)) {};

    Processor::DataSet data_set;
    unsigned data_sent1 = 0, data_received = 0, data_sent2 = 0;
};

struct Cluster::ClusterAllReduce : ClusterEvent {
    explicit ClusterAllReduce(ProcessingRulesPtr rp) :
        ClusterEvent(EventType::HTCAllReduce, std::move(rp)) {};

    Processor::DataSet data_set;
    DataPtr data_ptr;
    unsigned data_sent1 = 0, data_received = 0, data_sent2 = 0;
};

struct Cluster::ClusterReduceScatter : ClusterEvent {
    explicit ClusterReduceScatter(ProcessingRulesPtr rp) :
        ClusterEvent(EventType::HTCReduceScatter, std::move(rp)) {};

    Processor::DataSet data_set1;
    DataPtr data_ptr;
    Processor::DataSet data_set2;
    unsigned data_sent1 = 0, data_received = 0;
    bool scatter = false;
    unsigned data_sent2 = 0;
};

struct Cluster::ClusterAllToAll : ClusterEvent {
    explicit ClusterAllToAll(ProcessingRulesPtr rp) :
        ClusterEvent(EventType::HTCAllToAll, std::move(rp)) {};

    std::vector<Processor::DataSet> data_sets;
    unsigned got_data = 0, data_sent1 = 0, data_received_all = 0, data_sent2_all = 0;
};

OperationFun(ClusterBroadcast, broadcast)

OperationFun(ClusterScatter, scatter)

OperationFun(ClusterReduce, reduce)

OperationFun(ClusterAllGather, all_gather)

OperationFun(ClusterAllReduce, all_reduce)

OperationFun(ClusterReduceScatter, reduce_scatter)

OperationFun(ClusterAllToAll, all_to_all)
