//
// Created by taganyer on 25-6-20.
//

#include <tdcf/base/Errors.hpp>
#include <tdcf/base/types/Star.hpp>
#include <tdcf/cluster/star/StarCluster.hpp>

using namespace tdcf;

using namespace tdcf::star;

StarCluster::Scatter::Scatter(ProgressType type, uint32_t version, ProcessingRulesPtr rp) :
    EventProgress(OperationType::Scatter, type, version, std::move(rp)) {}

StatusFlag StarCluster::Scatter::create(ProcessingRulesPtr rp, Handle& handle) {
    uint32_t version = handle.create_progress_version();
    auto iter = handle.create_progress(
        std::make_unique<Scatter>(ProgressType::Root, version, std::move(rp)));

    auto& self = static_cast<Scatter&>(*iter->second);
    self._self = iter;

    MetaData meta = self.create_meta();
    meta.stage = C_Scatter::acquire_data;
    handle.acquire_data(iter, meta, self.rule);

    meta.stage = C_Scatter::send_rule;
    for (auto& id : handle.cluster_data<IdentityList>()) {
        StatusFlag flag = handle.send_progress_message(version, id, meta, self.rule);
        TDCF_CHECK_SUCCESS(flag)
    }

    return StatusFlag::Success;
}

StatusFlag StarCluster::Scatter::handle_event(const MetaData& meta,
                                              Variant& data, Handle& handle) {
    assert(meta.operation_type == OperationType::Scatter);
    if (meta.stage == C_Scatter::acquire_data) {
        return scatter_data(std::get<DataSet>(data), handle);
    }
    if (meta.stage == C_Scatter::scatter_data) {
        return send_data(std::get<DataSet>(data), handle);
    }
    if (meta.stage == C_Scatter::finish_ack) {
        ++_respond;
        if (_respond == handle.cluster_data<IdentityList>().size()) {
            rule->finish_callback();
            return StatusFlag::EventEnd;
        }
        return StatusFlag::Success;
    }
    TDCF_RAISE_ERROR(meta.stage error type)
}

StatusFlag StarCluster::Scatter::scatter_data(DataSet& dataset, Handle& handle) const {
    MetaData meta = create_meta();
    meta.stage = C_Scatter::scatter_data;
    handle.scatter_data(_self, meta, rule, handle.cluster_data<IdentityList>().size() + 1, std::move(dataset));
    return StatusFlag::Success;
}

StatusFlag StarCluster::Scatter::send_data(DataSet& set, Handle& handle) const {
    uint32_t total = handle.cluster_data<IdentityList>().size() + 1;
    uint32_t patch = set.size() / total;
    TDCF_CHECK_EXPR(patch > 0 && set.size() % total == 0)

    MetaData meta = create_meta();
    meta.stage = C_Scatter::send_data;
    meta.rest_data = patch;

    for (uint32_t i = 0; i < patch; ++i) {
        handle.store_data(rule, set[i]);
        --meta.rest_data;
        uint32_t j = i + patch;
        for (auto& id : handle.cluster_data<IdentityList>()) {
            StatusFlag flag = handle.send_progress_message(version, id, meta, std::move(set[j]));
            TDCF_CHECK_SUCCESS(flag)
            j += patch;
        }
    }

    return StatusFlag::Success;
}

StarCluster::ScatterAgent::ScatterAgent(uint32_t version, ProcessingRulesPtr rp, ProgressEventsMI iter) :
    Scatter(ProgressType::NodeRoot, version, std::move(rp)), _other(iter) {}

StatusFlag StarCluster::ScatterAgent::create(ProcessingRulesPtr rp, ProgressEventsMI other,
                                             Handle& handle, EventProgressAgent **agent_ptr) {
    uint32_t version = handle.create_progress_version();
    auto iter = handle.create_progress(
        std::make_unique<ScatterAgent>(version, std::move(rp), other));

    auto& self = static_cast<ScatterAgent&>(*iter->second);
    *agent_ptr = &self;
    self._self = iter;

    MetaData meta = self.create_meta();
    meta.stage = A_Scatter::send_rule;
    for (auto& id : handle.cluster_data<IdentityList>()) {
        StatusFlag flag = handle.send_progress_message(version, id, meta, self.rule);
        TDCF_CHECK_SUCCESS(flag)
    }

    return StatusFlag::Success;
}

StatusFlag StarCluster::ScatterAgent::handle_event(const MetaData& meta,
                                                   Variant& data, Handle& handle) {
    assert(meta.operation_type == OperationType::Scatter);
    if (meta.stage == Public_Scatter::agent_receive) {
        return scatter_data(std::get<DataSet>(data), handle);
    }
    if (meta.stage == A_Scatter::scatter_data) {
        return send_data(std::get<DataSet>(data), handle);
    }
    if (meta.stage == A_Scatter::finish_ack) {
        ++_respond;
        if (_respond == handle.cluster_data<IdentityList>().size()) {
            return close(handle);
        }
        return StatusFlag::Success;
    }
    TDCF_RAISE_ERROR(meta.stage error type)
}

StatusFlag StarCluster::ScatterAgent::proxy_event(const MetaData& meta,
                                                  Variant& data, Handle& handle) {
    return handle_event(meta, data, handle);
}

StatusFlag StarCluster::ScatterAgent::close(Handle& handle) const {
    MetaData meta = create_meta();
    meta.stage = Public_Scatter::agent_finish;
    handle.create_processor_event(_other, meta, nullptr);
    return StatusFlag::EventEnd;
}
