//
// Created by taganyer on 25-6-15.
//
#include <tdcf/base/Errors.hpp>
#include <tdcf/detail/NodeInformation.hpp>
#include <tdcf/node/agents/star/StarAgent.hpp>

using namespace tdcf;

StatusFlag StarAgent::init(const MetaData& meta, NodeInformation& info) {
    assert(meta.stage == Star::start);
    return StatusFlag::Success;
}

StatusFlag StarAgent::serialize(void *buffer, unsigned buffer_size) const {
    return StatusFlag::Success;
}

StatusFlag StarAgent::deserialize(const void *buffer, unsigned buffer_size) {
    return StatusFlag::Success;
}

SerializableType StarAgent::derived_type() const {
    return ClusterType::star;
}

unsigned StarAgent::serialize_size() const {
    return 0;
}

StatusFlag StarAgent::create_progress(const MetaData& meta, ProcessingRulesPtr& rule,
                                      NodeInformation& info) {
    switch (meta.operation_type) {
        case OperationType::Broadcast:
            return Broadcast::create(meta, rule, info);
        case OperationType::Scatter:
            return Scatter::create(meta, rule, info);
        case OperationType::Reduce:
            return Reduce::create(meta, rule, info);
        case OperationType::AllReduce:
            return AllReduce::create(meta, rule, info);
        case OperationType::ReduceScatter:
            return ReduceScatter::create(meta, rule, info);
        default:
            TDCF_RAISE_ERROR(error OperationType)
    }
}

StatusFlag StarAgent::end_agent(const MetaData& meta, NodeInformation& info) {
    assert(meta.stage == Star::close);
    assert(!info.delayed_message(info.root_id()));
    info.disconnect(info.root_id());
    return StatusFlag::Success;
}
