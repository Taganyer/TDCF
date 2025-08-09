//
// Created by taganyer on 25-7-8.
//

#include <tdcf/base/Errors.hpp>
#include <tdcf/base/types/Ring.hpp>
#include <tdcf/cluster/ring/RingCluster.hpp>

#include "tdcf/base/types/Star.hpp"

using namespace tdcf;

using namespace tdcf::ring;

RingCluster::AllReduce::AllReduce(ProgressType type, uint32_t version, ProcessingRulesPtr rp) :
    EventProgress(OperationType::AllReduce, type, version, std::move(rp)) {}

StatusFlag RingCluster::AllReduce::create(ProcessingRulesPtr rp, Handle& handle) {
    uint32_t version = handle.create_progress_version();
    auto iter = handle.create_progress(
        std::make_unique<AllReduce>(ProgressType::Root, version, std::move(rp)));

    auto& [send, receive, cluster_size] = handle.cluster_data<RingClusterData>();
    auto& self = static_cast<AllReduce&>(*iter->second);

    MetaData meta = self.create_meta();
    meta.stage = C_AllReduce::acquire_data;
    handle.acquire_data(iter, meta, self.rule);

    meta.stage = C_AllReduce::send_rule;
    StatusFlag flag = handle.send_progress_message(version, send, meta, self.rule);
    TDCF_CHECK_SUCCESS(flag)

    return StatusFlag::Success;
}

StatusFlag RingCluster::AllReduce::handle_event(const MetaData& meta,
                                                Variant& data, Handle& handle) {
    assert(meta.operation_type == OperationType::AllReduce);
    if (meta.stage == C_AllReduce::acquire_data) {
        return acquire_data(std::get<DataSet>(data), handle);
    }
    if (meta.stage == C_AllReduce::get_data2) {
        return send_data(std::get<DataPtr>(data), meta.rest_data, handle);
    }
    if (meta.stage == C_AllReduce::finish_ack) {
        return StatusFlag::EventEnd;
    }
    TDCF_RAISE_ERROR(meta.stage error type)
}

StatusFlag RingCluster::AllReduce::acquire_data(DataSet& dataset, Handle& handle) const {
    auto& [send, receive, cluster_size] = handle.cluster_data<RingClusterData>();
    MetaData meta = create_meta();
    meta.stage = C_AllReduce::send_data1;
    meta.rest_data = dataset.size();

    for (auto& data : dataset) {
        --meta.rest_data;
        StatusFlag flag = handle.send_progress_message(version, send, meta, std::move(data));
        TDCF_CHECK_SUCCESS(flag)
    }

    return StatusFlag::Success;
}

StatusFlag RingCluster::AllReduce::send_data(DataPtr& data, uint32_t rest_size, Handle& handle) const {
    handle.store_data(rule, data);
    auto& [send, receive, cluster_size] = handle.cluster_data<RingClusterData>();
    MetaData meta = create_meta();
    meta.stage = C_AllReduce::send_data2;
    meta.rest_data = rest_size;
    StatusFlag flag = handle.send_progress_message(version, send, meta, data);
    TDCF_CHECK_SUCCESS(flag)

    if (rest_size == 0) {
        meta.stage = C_AllReduce::finish;
        flag = handle.send_progress_message(version, send, meta, nullptr);
        TDCF_CHECK_SUCCESS(flag)
    }

    return StatusFlag::Success;
}

RingCluster::AllReduceAgent::AllReduceAgent(uint32_t version, ProcessingRulesPtr rp, ProgressEventsMI iter) :
    AllReduce(ProgressType::NodeRoot, version, std::move(rp)), _other(iter) {}

StatusFlag RingCluster::AllReduceAgent::create(ProcessingRulesPtr rp, ProgressEventsMI other,
                                               Handle& handle, EventProgressAgent **agent_ptr) {
    uint32_t version = handle.create_progress_version();
    auto iter = handle.create_progress(
        std::make_unique<AllReduceAgent>(version, std::move(rp), other));

    auto& [send, receive, cluster_size] = handle.cluster_data<RingClusterData>();
    auto& self = static_cast<AllReduceAgent&>(*iter->second);
    *agent_ptr = &self;

    MetaData meta = self.create_meta();
    meta.stage = A_AllReduce::acquire_data;
    handle.acquire_data(iter, meta, self.rule);

    meta.stage = A_AllReduce::send_rule;
    StatusFlag flag = handle.send_progress_message(version, send, meta, self.rule);
    TDCF_CHECK_SUCCESS(flag)
    return StatusFlag::Success;
}

StatusFlag RingCluster::AllReduceAgent::handle_event(const MetaData& meta,
                                                     Variant& data, Handle& handle) {
    assert(meta.operation_type == OperationType::AllReduce);
    assert(meta.operation_type == OperationType::AllReduce);
    if (meta.stage == A_AllReduce::acquire_data) {
        return acquire_data(std::get<DataSet>(data), handle);
    }
    if (meta.stage == A_AllReduce::get_data2) {
        return agent_get_data(std::get<DataPtr>(data), meta.rest_data, handle);
    }
    if (meta.stage == Public_AllReduce::agent_receive) {
        auto& set = std::get<DataSet>(data);
        uint32_t rest_size = set.size();
        for (auto& d : set) {
            --rest_size;
            send_data(d, rest_size, handle);
        }
        return StatusFlag::Success;
    }
    if (meta.stage == A_AllReduce::finish_ack) {
        return close(handle);
    }
    TDCF_RAISE_ERROR(meta.stage error type)
}

StatusFlag RingCluster::AllReduceAgent::proxy_event(const MetaData& meta,
                                                    Variant& data, Handle& handle) {
    return handle_event(meta, data, handle);
}

StatusFlag RingCluster::AllReduceAgent::agent_get_data(DataPtr& data,
                                                       uint32_t rest_size, Handle& handle) {
    _set.emplace_back(std::move(data));
    if (rest_size == 0) {
        MetaData meta = create_meta();
        meta.stage = Public_AllReduce::agent_send;
        handle.create_processor_event(_other, meta, std::move(_set));
    }
    return StatusFlag::Success;
}

StatusFlag RingCluster::AllReduceAgent::close(Handle& handle) const {
    MetaData meta = create_meta();
    meta.stage = Public_AllReduce::agent_finish;
    handle.create_processor_event(_other, meta, nullptr);
    return StatusFlag::EventEnd;
}
