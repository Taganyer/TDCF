//
// Created by taganyer on 25-7-23.
//

#include <tdcf/base/Errors.hpp>
#include <tdcf/base/types/DBT.hpp>
#include "tdcf/cluster/DBT/DBTCluster.hpp"

using namespace tdcf;

using namespace tdcf::dbt;

DBTCluster::Broadcast::Broadcast(ProgressType type, uint32_t version, ProcessingRulesPtr rp) :
    EventProgress(OperationType::Broadcast, type, version, std::move(rp)) {}

StatusFlag DBTCluster::Broadcast::create(ProcessingRulesPtr rp, Handle& handle) {
    uint32_t version = handle.create_progress_version();
    auto iter = handle.create_progress(
        std::make_unique<Broadcast>(ProgressType::Root, version, std::move(rp)));

    auto& [t1, t2, size] = handle.cluster_data<DBTClusterData>();
    auto& self = static_cast<Broadcast&>(*iter->second);

    MetaData meta = self.create_meta();
    meta.stage = C_Broadcast::acquire_data;
    handle.acquire_data(iter, meta, self.rule);

    meta.stage = C_Broadcast::send_rule;
    StatusFlag flag = handle.send_progress_message(version, t1, meta, self.rule);
    TDCF_CHECK_SUCCESS(flag)

    flag = handle.send_progress_message(version, t2, meta, self.rule);
    TDCF_CHECK_SUCCESS(flag)

    return StatusFlag::Success;
}

StatusFlag DBTCluster::Broadcast::handle_event(const MetaData& meta,
                                               Variant& data, Handle& handle) {
    assert(meta.operation_type == OperationType::Broadcast);
    if (meta.stage == C_Broadcast::acquire_data) {
        return send_data(std::get<DataSet>(data), handle);
    }
    if (meta.stage == C_Broadcast::finish_ack) {
        return StatusFlag::EventEnd;
    }
    TDCF_RAISE_ERROR(meta.stage error type)
}

StatusFlag DBTCluster::Broadcast::send_data(DataSet& dataset, Handle& handle) const {
    auto& [t1, t2, size] = handle.cluster_data<DBTClusterData>();
    MetaData meta = create_meta();
    meta.stage = C_Broadcast::send_data;
    if (dataset.size() == 1) dataset.emplace_back(std::make_shared<Data>());

    uint32_t t1_rest_data = (dataset.size() + 1) / 2, t2_rest_data = dataset.size() / 2;
    for (uint32_t i = 0; i < dataset.size(); ++i) {
        auto& data = dataset[i];
        meta.data1[1] = i & 1;
        StatusFlag flag;
        if (i & 1) {
            meta.rest_data = --t2_rest_data;
            meta.data1[0] = 0;
            flag = handle.send_progress_message(version, t2, meta, data);
        } else {
            meta.rest_data = --t1_rest_data;
            meta.data1[0] = 1;
            flag = handle.send_progress_message(version, t1, meta, data);
        }
        TDCF_CHECK_SUCCESS(flag)
    }

    return StatusFlag::Success;
}

DBTCluster::BroadcastAgent::BroadcastAgent(uint32_t version, ProcessingRulesPtr rp,
                                           ProgressEventsMI iter) :
    Broadcast(ProgressType::NodeRoot, version, std::move(rp)), _other(iter) {}

StatusFlag DBTCluster::BroadcastAgent::create(ProcessingRulesPtr rp, ProgressEventsMI other,
                                              Handle& handle, EventProgressAgent **agent_ptr) {
    uint32_t version = handle.create_progress_version();
    auto iter = handle.create_progress(
        std::make_unique<BroadcastAgent>(version, std::move(rp), other));

    auto& [t1, t2, size] = handle.cluster_data<DBTClusterData>();
    auto& self = static_cast<BroadcastAgent&>(*iter->second);
    *agent_ptr = &self;

    MetaData meta = self.create_meta();
    meta.stage = A_Broadcast::send_rule;

    StatusFlag flag = handle.send_progress_message(version, t1, meta, self.rule);
    TDCF_CHECK_SUCCESS(flag)

    flag = handle.send_progress_message(version, t2, meta, self.rule);
    TDCF_CHECK_SUCCESS(flag)

    return StatusFlag::Success;
}

StatusFlag DBTCluster::BroadcastAgent::handle_event(const MetaData& meta,
                                                    Variant& data, Handle& handle) {
    assert(meta.operation_type == OperationType::Broadcast);
    if (meta.stage == Public_Broadcast::agent_receive) {
        return send_data(std::get<DataSet>(data), handle);
    }
    if (meta.stage == A_Broadcast::finish_ack) {
        return close(handle);
    }
    TDCF_RAISE_ERROR(meta.stage error type)
}

StatusFlag DBTCluster::BroadcastAgent::proxy_event(const MetaData& meta,
                                                   Variant& data, Handle& handle) {
    return handle_event(meta, data, handle);
}

StatusFlag DBTCluster::BroadcastAgent::close(Handle& handle) const {
    MetaData meta = create_meta();
    meta.stage = Public_Broadcast::agent_finish;
    handle.create_processor_event(_other, meta, nullptr);
    return StatusFlag::EventEnd;
}
