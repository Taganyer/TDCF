//
// Created by taganyer on 25-7-28.
//

#include <tdcf/base/Errors.hpp>
#include <tdcf/base/types/DBT.hpp>
#include <tdcf/cluster/DBT/DBTCluster.hpp>

using namespace tdcf;

using namespace tdcf::dbt;

DBTCluster::Scatter::Scatter(ProgressType type, uint32_t version, ProcessingRulesPtr rp) :
    EventProgress(OperationType::Scatter, type, version, std::move(rp)) {}

StatusFlag DBTCluster::Scatter::create(ProcessingRulesPtr rp, Handle& handle) {
    uint32_t version = handle.create_progress_version();
    auto iter = handle.create_progress(
        std::make_unique<Scatter>(ProgressType::Root, version, std::move(rp)));

    auto& [t1, t2, size] = handle.cluster_data<DBTClusterData>();
    auto& self = static_cast<Scatter&>(*iter->second);
    self._self = iter;

    MetaData meta = self.create_meta();
    meta.stage = C_Scatter::acquire_data;
    handle.acquire_data(iter, meta, self.rule);

    meta.stage = C_Scatter::send_rule;
    StatusFlag flag = handle.send_progress_message(version, t1, meta, self.rule);
    TDCF_CHECK_SUCCESS(flag)

    flag = handle.send_progress_message(version, t2, meta, self.rule);
    TDCF_CHECK_SUCCESS(flag)

    return StatusFlag::Success;
}

StatusFlag DBTCluster::Scatter::handle_event(const MetaData& meta,
                                             Variant& data, Handle& handle) {
    assert(meta.operation_type == OperationType::Scatter);
    if (meta.stage == C_Scatter::acquire_data) {
        return scatter_data(std::get<DataSet>(data), handle);
    }
    if (meta.stage == C_Scatter::scatter_data) {
        return send_data(std::get<DataSet>(data), handle);
    }
    if (meta.stage == C_Scatter::finish_ack) {
        rule->finish_callback();
        return StatusFlag::EventEnd;
    }
    TDCF_RAISE_ERROR(meta.stage error type)
}

StatusFlag DBTCluster::Scatter::scatter_data(DataSet& dataset, Handle& handle) const {
    auto& [t1, t2, size] = handle.cluster_data<DBTClusterData>();
    MetaData meta = create_meta();
    meta.stage = C_Scatter::scatter_data;
    handle.scatter_data(_self, meta, rule, size + 1, std::move(dataset));
    return StatusFlag::Success;
}

StatusFlag DBTCluster::Scatter::send_data(DataSet& set, Handle& handle) const {
    auto& [t1, t2, size] = handle.cluster_data<DBTClusterData>();
    uint32_t patch = set.size() / (size + 1);
    TDCF_CHECK_EXPR(patch > 0 && set.size() % (size + 1) == 0);

    if (patch == 1) {
        DataSet tmp;
        tmp.reserve(set.size() * 2);
        for (auto& data : set) {
            tmp.emplace_back(std::move(data));
            tmp.emplace_back(DataPtr());
        }
        set = std::move(tmp);
        ++patch;
    }

    MetaData meta = create_meta();
    meta.stage = C_Scatter::send_data;

    StatusFlag flag;
    uint32_t t1_rest_data = (patch + 1) / 2, t2_rest_data = patch / 2;
    for (uint32_t i = 0; i < patch; ++i) {
        if (set[i]->derived_type() != 0) {
            handle.store_data(rule, set[i]);
        }
        if (i & 1) meta.rest_data = --t2_rest_data;
        else meta.rest_data = --t1_rest_data;
        for (uint32_t j = i + patch; j < set.size(); j += patch) {
            meta.serial = j / patch - 1;
            if (i & 1) {
                meta.data1[0] = 0;
                flag = handle.send_progress_message(version, t2, meta, set[j]);
                TDCF_CHECK_SUCCESS(flag)
            } else {
                meta.data1[0] = 1;
                flag = handle.send_progress_message(version, t1, meta, set[j]);
                TDCF_CHECK_SUCCESS(flag)
            }
        }
    }

    meta.stage = C_Scatter::finish_notify;

    meta.data1[0] = 1;
    flag = handle.send_progress_message(version, t1, meta, nullptr);
    TDCF_CHECK_SUCCESS(flag)
    meta.data1[0] = 0;
    flag = handle.send_progress_message(version, t2, meta, nullptr);
    TDCF_CHECK_SUCCESS(flag)

    return StatusFlag::Success;
}

DBTCluster::ScatterAgent::ScatterAgent(uint32_t version, ProcessingRulesPtr rp, ProgressEventsMI iter) :
    Scatter(ProgressType::NodeRoot, version, std::move(rp)), _other(iter) {}

StatusFlag DBTCluster::ScatterAgent::create(ProcessingRulesPtr rp, ProgressEventsMI other,
                                            Handle& handle, EventProgressAgent **agent_ptr) {
    uint32_t version = handle.create_progress_version();
    auto iter = handle.create_progress(
        std::make_unique<ScatterAgent>(version, std::move(rp), other));

    auto& [t1, t2, size] = handle.cluster_data<DBTClusterData>();
    auto& self = static_cast<ScatterAgent&>(*iter->second);
    *agent_ptr = &self;
    self._self = iter;

    MetaData meta = self.create_meta();
    meta.stage = A_Scatter::send_rule;
    StatusFlag flag = handle.send_progress_message(version, t1, meta, self.rule);
    TDCF_CHECK_SUCCESS(flag)
    flag = handle.send_progress_message(version, t2, meta, self.rule);
    TDCF_CHECK_SUCCESS(flag)

    return StatusFlag::Success;
}

StatusFlag DBTCluster::ScatterAgent::handle_event(const MetaData& meta,
                                                  Variant& data, Handle& handle) {
    assert(meta.operation_type == OperationType::Scatter);
    if (meta.stage == Public_Scatter::agent_receive) {
        return scatter_data(std::get<DataSet>(data), handle);
    }
    if (meta.stage == A_Scatter::scatter_data) {
        return send_data(std::get<DataSet>(data), handle);
    }
    if (meta.stage == A_Scatter::finish_ack) {
        return close(handle);
    }
    TDCF_RAISE_ERROR(meta.stage error type)
}

StatusFlag DBTCluster::ScatterAgent::proxy_event(const MetaData& meta,
                                                 Variant& data, Handle& handle) {
    return handle_event(meta, data, handle);
}

StatusFlag DBTCluster::ScatterAgent::close(Handle& handle) const {
    MetaData meta = create_meta();
    meta.stage = Public_Scatter::agent_finish;
    handle.create_processor_event(_other, meta, nullptr);
    return StatusFlag::EventEnd;
}
