//
// Created by taganyer on 25-6-16.
//

#include <tdcf/base/Errors.hpp>
#include <tdcf/cluster/StarCluster.hpp>

using namespace tdcf;

StarCluster::Broadcast::Broadcast(ProgressType type, uint32_t version, ProcessingRulesPtr rp) :
    EventProgress(OperationType::Broadcast, type, version, std::move(rp)) {}

StatusFlag StarCluster::Broadcast::create(ProcessingRulesPtr rp, Handle& handle) {
    uint32_t version = handle.create_conversation_version();
    auto iter = handle.create_progress(
        std::make_unique<Broadcast>(ProgressType::Root, version, std::move(rp)));

    auto& self = static_cast<Broadcast&>(*iter->second);
    self._self = iter;

    MetaData meta = self.create_meta();
    meta.stage = ClusterBroadcast::acquire_data;
    handle.acquire_data(iter, meta, self.rule);

    meta.stage = ClusterBroadcast::send_rule;
    for (auto& id : handle.identities) {
        StatusFlag flag = handle.start_progress_message(version, id, meta, self.rule);
        TDCF_CHECK_SUCCESS(flag)
    }

    return StatusFlag::Success;
}

StatusFlag StarCluster::Broadcast::handle_event(const MetaData& meta,
                                                Variant& data, Handle& handle) {
    assert(meta.operation_type == OperationType::Broadcast);
    if (meta.stage == ClusterBroadcast::acquire_data) {
        assert(_sent == 0);
        return send_data(std::get<DataPtr>(data), handle);
    }
    if (meta.stage == ClusterBroadcast::finish_ack) {
        ++_respond;
        if (_respond == handle.cluster_size()) {
            rule->finish_callback();
            handle.close_conversation(version);
            return StatusFlag::EventEnd;
        }
        return StatusFlag::Success;
    }
    TDCF_RAISE_ERROR(meta.stage error type)
}

StatusFlag StarCluster::Broadcast::send_data(DataPtr& data, Handle& handle) const {
    MetaData meta = create_meta();
    assert(handle.cluster_size() == handle.identities.size());
    for (auto& id : handle.identities) {
        meta.serial = _sent;
        StatusFlag flag = handle.send_progress_message(version, id, meta, data);
        TDCF_CHECK_SUCCESS(flag)
    }
    return StatusFlag::Success;
}

StarCluster::BroadcastAgent::BroadcastAgent(uint32_t version, ProcessingRulesPtr rp,
                                            ProgressEventsMI iter) :
    Broadcast(ProgressType::NodeRoot, version, std::move(rp)), _other(iter) {}

StatusFlag StarCluster::BroadcastAgent::create(ProcessingRulesPtr rp, ProgressEventsMI other,
                                               Handle& handle, EventProgressAgent **agent_ptr) {
    uint32_t version = handle.create_conversation_version();
    auto iter = handle.create_progress(
        std::make_unique<BroadcastAgent>(version, std::move(rp), other));

    auto& self = static_cast<BroadcastAgent&>(*iter->second);
    *agent_ptr = &self;
    self._self = iter;

    MetaData meta = self.create_meta();
    meta.stage = ClusterBroadcast::send_rule;
    for (auto& id : handle.identities) {
        StatusFlag flag = handle.start_progress_message(version, id, meta, self.rule);
        TDCF_CHECK_SUCCESS(flag)
    }

    return StatusFlag::Success;
}

StatusFlag StarCluster::BroadcastAgent::handle_event(const MetaData& meta,
                                                     Variant& data, Handle& handle) {
    assert(meta.operation_type == OperationType::Broadcast);
    if (meta.stage == AgentBroadcast::get_data) {
        assert(_sent == 0);
        handle.store_data(rule, std::get<DataPtr>(data));
        return send_data(std::get<DataPtr>(data), handle);
    }
    if (meta.stage == AgentBroadcast::finish_ack) {
        ++_respond;
        if (_respond == handle.cluster_size()) {
            return close(handle);
        }
        return StatusFlag::Success;
    }
    TDCF_RAISE_ERROR(meta.stage error type)
}

StatusFlag StarCluster::BroadcastAgent::proxy_event(const MetaData& meta,
                                                    Variant& data, Handle& handle) {
    return handle_event(meta, data, handle);
}

StatusFlag StarCluster::BroadcastAgent::close(Handle& handle) const {
    MetaData meta = create_meta();
    meta.stage = AgentBroadcast::finish;
    handle.create_processor_event(_other, meta, nullptr);
    handle.close_conversation(version);
    return StatusFlag::EventEnd;
}
