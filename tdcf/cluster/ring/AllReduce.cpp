//
// Created by taganyer on 25-7-8.
//

#include <tdcf/base/Errors.hpp>
#include <tdcf/cluster/ring/RingCluster.hpp>

using namespace tdcf;

RingCluster::AllReduce::AllReduce(ProgressType type, uint32_t version, ProcessingRulesPtr rp) :
    EventProgress(OperationType::AllReduce, type, version, std::move(rp)) {}

StatusFlag RingCluster::AllReduce::create(ProcessingRulesPtr rp, Handle& handle) {
}

StatusFlag RingCluster::AllReduce::handle_event(const MetaData& meta, Variant& data, Handle& handle) {
}

StatusFlag RingCluster::AllReduce::acquire_data(const MetaData& meta, DataPtr& data, Handle& handle) {
}

StatusFlag RingCluster::AllReduce::send_data(DataPtr& data, Handle& handle) {
}

