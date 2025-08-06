//
// Created by taganyer on 25-6-15.
//

#include <tdcf/base/Errors.hpp>
#include <tdcf/handle/Handle.hpp>
#include <tdcf/node/agents/star/StarAgent.hpp>

using namespace tdcf;

void StarAgent::init(const IdentityPtr& from_id, const MetaData& meta, Handle& handle) {
    assert(meta.stage == Star::start);
    handle.create_agent_data<IdentityPtr>(from_id);
}

void StarAgent::agent_start(Handle& handle) {
    auto& root = handle.agent_data<IdentityPtr>();
    MetaData meta;
    meta.stage = Star::respond;
    StatusFlag flag = handle.send_message(root, meta, nullptr);
    TDCF_CHECK_SUCCESS(flag)
}

StatusFlag StarAgent::handle_disconnect(const IdentityPtr& id, Handle& handle) {
    auto& root = handle.agent_data<IdentityPtr>();
    assert(equal_to(root, id));
    handle.disconnect(root);
    return StatusFlag::ClusterOffline;
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
