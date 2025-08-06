//
// Created by taganyer on 25-7-31.
//

#include <tdcf/base/Errors.hpp>
#include <tdcf/base/types/DBT.hpp>
#include <tdcf/cluster/DBT/DBTCluster.hpp>

using namespace tdcf;

using namespace tdcf::dbt;

DBTCluster::AllReduce::AllReduce(ProgressType type, uint32_t version, ProcessingRulesPtr rp) :
    EventProgress(OperationType::AllReduce, type, version, std::move(rp)) {}

StatusFlag DBTCluster::AllReduce::create(ProcessingRulesPtr rp, Handle& handle) {
    uint32_t version = handle.create_progress_version();
    auto iter = handle.create_progress(
        std::make_unique<AllReduce>(ProgressType::Root, version, std::move(rp)));

    auto& [t1, t2, size] = handle.cluster_data<DBTClusterData>();

    auto& self = static_cast<AllReduce&>(*iter->second);
    self._self = iter;

    MetaData meta = self.create_meta();
    meta.stage = C_AllReduce::self_acquire_data;
    handle.acquire_data(iter, meta, self.rule);

    meta.stage = C_AllReduce::send_rule;
    StatusFlag flag = handle.send_progress_message(version, t1, meta, self.rule);
    TDCF_CHECK_SUCCESS(flag)

    flag = handle.send_progress_message(version, t2, meta, self.rule);
    TDCF_CHECK_SUCCESS(flag)

    return StatusFlag::Success;
}

StatusFlag DBTCluster::AllReduce::handle_event(const MetaData& meta,
                                               Variant& data, Handle& handle) {
    assert(meta.operation_type == OperationType::AllReduce);
    if (meta.stage == C_AllReduce::self_acquire_data) {
        auto& set = std::get<DataSet>(data);
        uint32_t rest_size = set.size();
        for (auto& d : set) {
            --rest_size;
            acquire_data(d, rest_size, handle);
        }
        return StatusFlag::Success;
    }
    if (meta.stage == C_AllReduce::acquire_data) {
        return acquire_data(std::get<DataPtr>(data), meta.rest_data, handle);
    }
    if (meta.stage == C_AllReduce::reduce_data) {
        return send_data(std::get<DataSet>(data), handle);
    }
    if (meta.stage == C_AllReduce::finish_ack) {
        rule->finish_callback();
        return StatusFlag::EventEnd;
    }
    if (meta.stage == C_AllReduce::send_rule) {
        return StatusFlag::Success;
    }
    TDCF_RAISE_ERROR(meta.stage error type)
}

StatusFlag DBTCluster::AllReduce::acquire_data(DataPtr& data,
                                               uint32_t rest_size, Handle& handle) {
    if (data->derived_type() != 0) _set.emplace_back(std::move(data));
    if (rest_size == 0 && ++_received == 3) {
        MetaData meta = create_meta();
        meta.stage = C_AllReduce::reduce_data;
        handle.reduce_data(_self, meta, rule, std::move(_set));
    }
    return StatusFlag::Success;
}

StatusFlag DBTCluster::AllReduce::send_data(DataSet& dataset, Handle& handle) const {
    auto& [t1, t2, size] = handle.cluster_data<DBTClusterData>();
    MetaData meta = create_meta();
    meta.stage = C_AllReduce::send_data;
    if (dataset.size() == 1) dataset.emplace_back(DataPtr());

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

DBTCluster::AllReduceAgent::AllReduceAgent(uint32_t version, ProcessingRulesPtr rp, ProgressEventsMI iter) :
    AllReduce(ProgressType::NodeRoot, version, std::move(rp)), _other(iter) {}

StatusFlag DBTCluster::AllReduceAgent::create(ProcessingRulesPtr rp, ProgressEventsMI other,
                                              Handle& handle, EventProgressAgent **agent_ptr) {
    uint32_t version = handle.create_progress_version();
    auto iter = handle.create_progress(
        std::make_unique<AllReduceAgent>(version, std::move(rp), other));

    auto& [t1, t2, size] = handle.cluster_data<DBTClusterData>();

    auto& self = static_cast<AllReduceAgent&>(*iter->second);
    *agent_ptr = &self;
    self._self = iter;

    MetaData meta = self.create_meta();
    meta.stage = A_AllReduce::self_acquire_data;
    handle.acquire_data(iter, meta, self.rule);

    meta.stage = A_AllReduce::send_rule;
    StatusFlag flag = handle.send_progress_message(version, t1, meta, self.rule);
    TDCF_CHECK_SUCCESS(flag)

    flag = handle.send_progress_message(version, t2, meta, self.rule);
    TDCF_CHECK_SUCCESS(flag)

    return StatusFlag::Success;
}

StatusFlag DBTCluster::AllReduceAgent::handle_event(const MetaData& meta,
                                                    Variant& data, Handle& handle) {
    assert(meta.operation_type == OperationType::AllReduce);
    if (meta.stage == A_AllReduce::self_acquire_data) {
        auto& set = std::get<DataSet>(data);
        uint32_t rest_size = set.size();
        for (auto& d : set) {
            --rest_size;
            acquire_data(d, rest_size, handle);
        }
        return StatusFlag::Success;
    }
    if (meta.stage == A_AllReduce::acquire_data1) {
        return acquire_data(std::get<DataPtr>(data), meta.rest_data, handle);
    }
    if (meta.stage == A_AllReduce::reduce_data) {
        return reduce_data(std::get<DataSet>(data), handle);
    }
    if (meta.stage == Public_AllReduce::agent_receive) {
        return send_data(std::get<DataSet>(data), handle);
    }
    if (meta.stage == A_AllReduce::finish_ack) {
        return close(handle);
    }
    if (meta.stage == C_AllReduce::send_rule) {
        return StatusFlag::Success;
    }
    TDCF_RAISE_ERROR(meta.stage error type)
}

StatusFlag DBTCluster::AllReduceAgent::proxy_event(const MetaData& meta,
                                                   Variant& data, Handle& handle) {
    return handle_event(meta, data, handle);
}

StatusFlag DBTCluster::AllReduceAgent::reduce_data(DataSet& dataset, Handle& handle) const {
    MetaData meta = create_meta();
    meta.stage = Public_AllReduce::agent_send;
    handle.create_processor_event(_other, meta, std::move(dataset));
    return StatusFlag::Success;
}

StatusFlag DBTCluster::AllReduceAgent::close(Handle& handle) const {
    MetaData meta = create_meta();
    meta.stage = Public_AllReduce::agent_finish;
    handle.create_processor_event(_other, meta, nullptr);
    return StatusFlag::EventEnd;
}
