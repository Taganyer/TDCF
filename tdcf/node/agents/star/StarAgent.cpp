//
// Created by taganyer on 25-6-15.
//

#include <tdcf/base/Errors.hpp>
#include <tdcf/handle/Handle.hpp>
#include <tdcf/node/agents/star/StarAgent.hpp>

using namespace tdcf;

StatusFlag StarAgent::init(const IdentityPtr& from_id, const MetaData& meta, Handle& handle) {
    assert(meta.stage == Star::start);
    handle.create_agent_data<IdentityPtr>(from_id);
    return StatusFlag::Success;
}

bool StarAgent::serialize(void *buffer, uint32_t buffer_size) const {
    return true;
}

bool StarAgent::deserialize(const void *buffer, uint32_t buffer_size) {
    return true;
}

SerializableType StarAgent::derived_type() const {
    return ClusterType::star;
}

uint32_t StarAgent::serialize_size() const {
    return 0;
}

StatusFlag StarAgent::create_progress(uint32_t version, const MetaData& meta,
                                      ProcessingRulesPtr& rule, Handle& handle) {
    switch (meta.operation_type) {
        case OperationType::Broadcast:
            return Broadcast::create(version, meta, rule, handle);
        case OperationType::Scatter:
            return Scatter::create(version, meta, rule, handle);
        case OperationType::Reduce:
            return Reduce::create(version, meta, rule, handle);
        case OperationType::AllReduce:
            return AllReduce::create(version, meta, rule, handle);
        case OperationType::ReduceScatter:
            return ReduceScatter::create(version, meta, rule, handle);
        default:
            TDCF_RAISE_ERROR(error OperationType)
    }
}

StatusFlag StarAgent::end_agent(const MetaData& meta, Handle& handle) {
    assert(meta.stage == Star::close);
    assert(!handle.delayed_message(handle.agent_data<IdentityPtr>()));
    handle.disconnect(handle.agent_data<IdentityPtr>());
    return StatusFlag::Success;
}
