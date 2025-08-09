//
// Created by taganyer on 25-6-16.
//

#include <tdcf/base/Errors.hpp>
#include <tdcf/base/types/Star.hpp>
#include <tdcf/cluster/star/StarCluster.hpp>

using namespace tdcf;

using namespace tdcf::star;

StarCluster::Broadcast::Broadcast(ProgressType type, uint32_t version, ProcessingRulesPtr rp) :
    EventProgress(OperationType::Broadcast, type, version, std::move(rp)) {}

StatusFlag StarCluster::Broadcast::create(ProcessingRulesPtr rp, Handle& handle) {
    uint32_t version = handle.create_progress_version();
    auto iter = handle.create_progress(
        std::make_unique<Broadcast>(ProgressType::Root, version, std::move(rp)));

    auto& self = static_cast<Broadcast&>(*iter->second);

    MetaData meta = self.create_meta();
    meta.stage = C_Broadcast::acquire_data;
    handle.acquire_data(iter, meta, self.rule);

    meta.stage = C_Broadcast::send_rule;
    for (auto& id : handle.cluster_data<IdentityList>()) {
        StatusFlag flag = handle.send_progress_message(version, id, meta, self.rule);
        TDCF_CHECK_SUCCESS(flag)
    }

    return StatusFlag::Success;
}

StatusFlag StarCluster::Broadcast::handle_event(const MetaData& meta,
                                                Variant& data, Handle& handle) {
    assert(meta.operation_type == OperationType::Broadcast);
    if (meta.stage == C_Broadcast::acquire_data) {
        auto& set = std::get<DataSet>(data);
        uint32_t rest_size = set.size();
        for (auto& d : set) {
            --rest_size;
            send_data(d, rest_size, handle);
        }
        return StatusFlag::Success;
    }
    if (meta.stage == C_Broadcast::finish_ack) {
        ++_respond;
        if (_respond == handle.cluster_data<IdentityList>().size()) {
            return StatusFlag::EventEnd;
        }
        return StatusFlag::Success;
    }
    TDCF_RAISE_ERROR(meta.stage error type)
}

void StarCluster::Broadcast::send_data(DataPtr& data, uint32_t rest_size, Handle& handle) const {
    MetaData meta = create_meta();
    meta.stage = C_Broadcast::send_data;
    meta.rest_data = rest_size;

    for (auto& id : handle.cluster_data<IdentityList>()) {
        StatusFlag flag = handle.send_progress_message(version, id, meta, data);
        TDCF_CHECK_SUCCESS(flag)
    }
}

StarCluster::BroadcastAgent::BroadcastAgent(uint32_t version, ProcessingRulesPtr rp,
                                            ProgressEventsMI iter) :
    Broadcast(ProgressType::NodeRoot, version, std::move(rp)), _other(iter) {}

StatusFlag StarCluster::BroadcastAgent::create(ProcessingRulesPtr rp, ProgressEventsMI other,
                                               Handle& handle, EventProgressAgent **agent_ptr) {
    uint32_t version = handle.create_progress_version();
    auto iter = handle.create_progress(
        std::make_unique<BroadcastAgent>(version, std::move(rp), other));

    auto& self = static_cast<BroadcastAgent&>(*iter->second);
    *agent_ptr = &self;

    MetaData meta = self.create_meta();
    meta.stage = C_Broadcast::send_rule;
    for (auto& id : handle.cluster_data<IdentityList>()) {
        StatusFlag flag = handle.send_progress_message(version, id, meta, self.rule);
        TDCF_CHECK_SUCCESS(flag)
    }

    return StatusFlag::Success;
}

StatusFlag StarCluster::BroadcastAgent::handle_event(const MetaData& meta,
                                                     Variant& data, Handle& handle) {
    assert(meta.operation_type == OperationType::Broadcast);
    if (meta.stage == Public_Broadcast::agent_receive) {
        auto& set = std::get<DataSet>(data);
        uint32_t rest_size = set.size();
        for (auto& d : set) {
            --rest_size;
            handle.store_data(rule, d);
            send_data(d, rest_size, handle);
        }
        return StatusFlag::Success;
    }
    if (meta.stage == A_Broadcast::finish_ack) {
        ++_respond;
        if (_respond == handle.cluster_data<IdentityList>().size()) {
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
    meta.stage = Public_Broadcast::agent_finish;
    handle.create_processor_event(_other, meta, nullptr);
    return StatusFlag::EventEnd;
}
