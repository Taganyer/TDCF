//
// Created by taganyer on 25-6-22.
//

#include <tdcf/base/Errors.hpp>
#include <tdcf/base/types/Star.hpp>
#include <tdcf/cluster/star/StarCluster.hpp>

using namespace tdcf;

using namespace tdcf::star;

StarCluster::AllReduce::AllReduce(ProgressType type, uint32_t version, ProcessingRulesPtr rp) :
    EventProgress(OperationType::AllReduce, type, version, std::move(rp)) {}

StatusFlag StarCluster::AllReduce::create(ProcessingRulesPtr rp, Handle& handle) {
    uint32_t version = handle.create_progress_version();
    auto iter = handle.create_progress(
        std::make_unique<AllReduce>(ProgressType::Root, version, std::move(rp)));

    auto& self = static_cast<AllReduce&>(*iter->second);
    self._self = iter;

    MetaData meta = self.create_meta();
    meta.stage = C_AllReduce::self_acquire_data;
    handle.acquire_data(iter, meta, self.rule);

    meta.stage = C_AllReduce::send_rule;
    uint32_t serial = 0;
    for (auto& id : handle.cluster_data<IdentityList>()) {
        meta.serial = ++serial;
        StatusFlag flag = handle.send_progress_message(version, id, meta, self.rule);
        TDCF_CHECK_SUCCESS(flag)
    }

    return StatusFlag::Success;
}

StatusFlag StarCluster::AllReduce::handle_event(const MetaData& meta,
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
        ++_respond;
        if (_respond == handle.cluster_data<IdentityList>().size()) {
            rule->finish_callback();
            return StatusFlag::EventEnd;
        }
        return StatusFlag::Success;
    }
    TDCF_RAISE_ERROR(meta.stage error type)
}

StatusFlag StarCluster::AllReduce::acquire_data(DataPtr& data,
                                                uint32_t rest_size, Handle& handle) {
    _set.emplace_back(std::move(data));
    if (rest_size == 0) ++_received;
    if (_received == handle.cluster_data<IdentityList>().size() + 1) {
        MetaData new_meta = create_meta();
        new_meta.stage = C_AllReduce::reduce_data;
        handle.reduce_data(_self, new_meta, rule, std::move(_set));
    }
    return StatusFlag::Success;
}

StatusFlag StarCluster::AllReduce::send_data(DataSet& dataset, Handle& handle) const {
    MetaData meta = create_meta();
    meta.stage = C_AllReduce::send_data;
    meta.rest_data = dataset.size();

    for (auto& data : dataset) {
        --meta.rest_data;
        handle.store_data(rule, data);
        for (auto& id : handle.cluster_data<IdentityList>()) {
            StatusFlag flag = handle.send_progress_message(version, id, meta, data);
            TDCF_CHECK_SUCCESS(flag)
        }
    }

    return StatusFlag::Success;
}

StarCluster::AllReduceAgent::AllReduceAgent(uint32_t version, ProcessingRulesPtr rp, ProgressEventsMI iter) :
    AllReduce(ProgressType::NodeRoot, version, std::move(rp)), _other(iter) {}

StatusFlag StarCluster::AllReduceAgent::create(ProcessingRulesPtr rp, ProgressEventsMI other,
                                               Handle& handle, EventProgressAgent **agent_ptr) {
    uint32_t version = handle.create_progress_version();
    auto iter = handle.create_progress(
        std::make_unique<AllReduceAgent>(version, std::move(rp), other));

    auto& self = static_cast<AllReduceAgent&>(*iter->second);
    *agent_ptr = &self;
    self._self = iter;

    MetaData meta = self.create_meta();
    meta.stage = A_AllReduce::self_acquire_data;
    handle.acquire_data(iter, meta, self.rule);

    meta.stage = A_AllReduce::send_rule;
    uint32_t serial = 0;
    for (auto& id : handle.cluster_data<IdentityList>()) {
        meta.serial = ++serial;
        StatusFlag flag = handle.send_progress_message(version, id, meta, self.rule);
        TDCF_CHECK_SUCCESS(flag)
    }

    return StatusFlag::Success;
}

StatusFlag StarCluster::AllReduceAgent::handle_event(const MetaData& meta,
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
        ++_respond;
        if (_respond == handle.cluster_data<IdentityList>().size()) {
            return close(handle);
        }
        return StatusFlag::Success;
    }
    TDCF_RAISE_ERROR(meta.stage error type)
}

StatusFlag StarCluster::AllReduceAgent::proxy_event(const MetaData& meta,
                                                    Variant& data, Handle& handle) {
    return handle_event(meta, data, handle);
}

StatusFlag StarCluster::AllReduceAgent::reduce_data(DataSet& dataset, Handle& handle) const {
    MetaData meta = create_meta();
    meta.stage = Public_AllReduce::agent_send;
    handle.create_processor_event(_other, meta, std::move(dataset));
    return StatusFlag::Success;
}

StatusFlag StarCluster::AllReduceAgent::close(Handle& handle) const {
    MetaData meta = create_meta();
    meta.stage = Public_AllReduce::agent_finish;
    handle.create_processor_event(_other, meta, nullptr);
    return StatusFlag::EventEnd;
}
