//
// Created by taganyer on 25-7-30.
//

#include <tdcf/base/Errors.hpp>
#include <tdcf/base/types/DBT.hpp>
#include <tdcf/cluster/DBT/DBTCluster.hpp>

using namespace tdcf;

using namespace tdcf::dbt;

DBTCluster::Reduce::Reduce(ProgressType type, uint32_t version, ProcessingRulesPtr rp) :
    EventProgress(OperationType::Reduce, type, version, std::move(rp)) {}

StatusFlag DBTCluster::Reduce::create(ProcessingRulesPtr rp, Handle& handle) {
    uint32_t version = handle.create_progress_version();
    auto iter = handle.create_progress(
        std::make_unique<Reduce>(ProgressType::Root, version, std::move(rp)));

    auto& [t1, t2, size] = handle.cluster_data<DBTClusterData>();

    auto& self = static_cast<Reduce&>(*iter->second);
    self._self = iter;

    MetaData meta = self.create_meta();
    meta.stage = C_Reduce::self_acquire_data;
    handle.acquire_data(iter, meta, self.rule);

    meta.stage = C_Reduce::send_rule;
    StatusFlag flag = handle.send_progress_message(version, t1, meta, self.rule);
    TDCF_CHECK_SUCCESS(flag)

    flag = handle.send_progress_message(version, t2, meta, self.rule);
    TDCF_CHECK_SUCCESS(flag)

    return StatusFlag::Success;
}

StatusFlag DBTCluster::Reduce::handle_event(const MetaData& meta,
                                            Variant& data, Handle& handle) {
    assert(meta.operation_type == OperationType::Reduce);
    if (meta.stage == C_Reduce::self_acquire_data) {
        auto& set = std::get<DataSet>(data);
        uint32_t rest_size = set.size();
        for (auto& d : set) {
            --rest_size;
            acquire_data(d, rest_size, handle);
        }
        return StatusFlag::Success;
    }
    if (meta.stage == C_Reduce::acquire_data) {
        return acquire_data(std::get<DataPtr>(data), meta.rest_data, handle);
    }
    if (meta.stage == C_Reduce::reduce_data) {
        for (auto& data_ptr : std::get<DataSet>(data)) {
            handle.store_data(rule, data_ptr);
        }
        rule->finish_callback();
        return StatusFlag::EventEnd;
    }
    if (meta.stage == C_Reduce::send_rule) {
        return StatusFlag::Success;
    }
    TDCF_RAISE_ERROR(meta.stage error type)
}

StatusFlag DBTCluster::Reduce::acquire_data(DataPtr& data,
                                            uint32_t rest_size, Handle& handle) {
    if (data->derived_type() != 0) _set.emplace_back(std::move(data));
    if (rest_size == 0 && ++_receive == 3) {
        MetaData meta = create_meta();
        meta.stage = C_Reduce::reduce_data;
        handle.reduce_data(_self, meta, rule, std::move(_set));
    }
    return StatusFlag::Success;
}

DBTCluster::ReduceAgent::ReduceAgent(uint32_t version, ProcessingRulesPtr rp, ProgressEventsMI iter) :
    Reduce(ProgressType::NodeRoot, version, std::move(rp)), _other(iter) {}

StatusFlag DBTCluster::ReduceAgent::create(ProcessingRulesPtr rp, ProgressEventsMI other,
                                           Handle& handle, EventProgressAgent **agent_ptr) {
    uint32_t version = handle.create_progress_version();
    auto iter = handle.create_progress(
        std::make_unique<ReduceAgent>(version, std::move(rp), other));

    auto& [t1, t2, size] = handle.cluster_data<DBTClusterData>();

    auto& self = static_cast<ReduceAgent&>(*iter->second);
    *agent_ptr = &self;
    self._self = iter;

    MetaData meta = self.create_meta();
    meta.stage = A_Reduce::self_acquire_data;
    handle.acquire_data(iter, meta, self.rule);

    meta.stage = A_Reduce::send_rule;
    StatusFlag flag = handle.send_progress_message(version, t1, meta, self.rule);
    TDCF_CHECK_SUCCESS(flag)

    flag = handle.send_progress_message(version, t2, meta, self.rule);
    TDCF_CHECK_SUCCESS(flag)

    return StatusFlag::Success;
}

StatusFlag DBTCluster::ReduceAgent::handle_event(const MetaData& meta,
                                                 Variant& data, Handle& handle) {
    assert(meta.operation_type == OperationType::Reduce);
    if (meta.stage == A_Reduce::self_acquire_data) {
        auto& set = std::get<DataSet>(data);
        uint32_t rest_size = set.size();
        for (auto& d : set) {
            --rest_size;
            acquire_data(d, rest_size, handle);
        }
        return StatusFlag::Success;
    }
    if (meta.stage == A_Reduce::acquire_data) {
        return acquire_data(std::get<DataPtr>(data), meta.rest_data, handle);
    }
    if (meta.stage == A_Reduce::reduce_data) {
        return close(std::get<DataSet>(data), handle);
    }
    if (meta.stage == C_Reduce::send_rule) {
        return StatusFlag::Success;
    }
    TDCF_RAISE_ERROR(meta.stage error type)
}

StatusFlag DBTCluster::ReduceAgent::proxy_event(const MetaData& meta,
                                                Variant& data, Handle& handle) {
    return handle_event(meta, data, handle);
}

StatusFlag DBTCluster::ReduceAgent::close(DataSet& dataset, Handle& handle) const {
    MetaData meta = create_meta();
    meta.stage = Public_Reduce::agent_send;
    handle.create_processor_event(_other, meta, std::move(dataset));
    return StatusFlag::EventEnd;
}
