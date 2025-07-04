//
// Created by taganyer on 25-6-20.
//

#include <tdcf/base/Errors.hpp>
#include <tdcf/cluster/StarCluster.hpp>

using namespace tdcf;

StarCluster::Scatter::Scatter(ProgressType type, uint32_t version, ProcessingRulesPtr rp) :
    EventProgress(OperationType::Scatter, type, version, std::move(rp)) {}

StatusFlag StarCluster::Scatter::create(ProcessingRulesPtr rp, Handle& handle) {
    uint32_t version = handle.create_conversation_version();
    auto iter = handle.create_progress(
        std::make_unique<Scatter>(ProgressType::Root, version, std::move(rp)));

    auto& self = static_cast<Scatter&>(*iter->second);
    self._self = iter;

    MetaData meta = self.create_meta();
    meta.stage = ClusterScatter::acquire_data;
    handle.acquire_data(iter, meta, self.rule);

    meta.stage = ClusterScatter::send_rule;
    for (auto& id : handle.identities) {
        StatusFlag flag = handle.start_progress_message(version, id, meta, self.rule);
        TDCF_CHECK_SUCCESS(flag)
    }

    return StatusFlag::Success;
}

StatusFlag StarCluster::Scatter::handle_event(const MetaData& meta,
                                              Variant& data, Handle& handle) {
    assert(meta.operation_type == OperationType::Scatter);
    if (meta.stage == ClusterScatter::acquire_data) {
        assert(_sent == 0);
        return scatter_data(std::get<DataPtr>(data), handle);
    }
    if (meta.stage == ClusterScatter::scatter_data) {
        assert(_sent == 0);
        return send_data(0, std::get<DataSet>(data), handle);
    }
    if (meta.stage == ClusterScatter::finish_ack) {
        assert(meta.progress_type == ProgressType::Node);
        ++_respond;
        if (_respond == handle.cluster_size()) {
            rule->finish_callback();
        }
        return StatusFlag::EventEnd;
    }
    TDCF_RAISE_ERROR(meta.stage error type)
}

StatusFlag StarCluster::Scatter::scatter_data(DataPtr& data, Handle& handle) const {
    MetaData meta = create_meta();
    meta.stage = ClusterScatter::scatter_data;
    handle.scatter_data(_self, meta, rule, handle.cluster_size(), data);
    return StatusFlag::Success;
}

StatusFlag StarCluster::Scatter::send_data(unsigned offset,
                                           DataSet& set, Handle& handle) {
    TDCF_CHECK_EXPR(set.size() == handle.cluster_size());
    MetaData meta = create_meta();
    meta.stage = ClusterScatter::send_data;

    assert(handle.cluster_size() == handle.identities.size());
    for (auto& id : handle.identities) {
        meta.serial = _sent++;
        StatusFlag flag = handle.send_progress_message(version, id, meta, std::move(set[_sent + offset]));
        TDCF_CHECK_SUCCESS(flag)
    }
    return StatusFlag::Success;
}

StarCluster::ScatterAgent::ScatterAgent(uint32_t version, ProcessingRulesPtr rp, ProgressEventsMI iter) :
    Scatter(ProgressType::NodeRoot, version, std::move(rp)), _other(iter) {}

StatusFlag StarCluster::ScatterAgent::create(ProcessingRulesPtr rp, ProgressEventsMI other,
                                             Handle& handle, EventProgressAgent **agent_ptr) {
    uint32_t version = handle.create_conversation_version();
    auto iter = handle.create_progress(
        std::make_unique<ScatterAgent>(version, std::move(rp), other));

    auto& self = static_cast<ScatterAgent&>(*iter->second);
    *agent_ptr = &self;
    self._self = iter;

    MetaData meta = self.create_meta();
    meta.stage = AgentScatter::send_rule;
    for (auto& id : handle.identities) {
        StatusFlag flag = handle.start_progress_message(version, id, meta, self.rule);
        TDCF_CHECK_SUCCESS(flag)
    }

    return StatusFlag::Success;
}

StatusFlag StarCluster::ScatterAgent::handle_event(const MetaData& meta,
                                                   Variant& data, Handle& handle) {
    assert(meta.operation_type == OperationType::Scatter);
    if (meta.stage == AgentScatter::get_data) {
        assert(_sent == 0);
        auto& set = std::get<DataSet>(data);
        handle.store_data(rule, set.front());
        return send_data(1, set, handle);
    }
    if (meta.stage == AgentScatter::finish_ack) {
        ++_respond;
        if (_respond == handle.cluster_size()) {
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
    meta.stage = AgentScatter::finish;
    handle.create_processor_event(_other, meta, nullptr);
    return StatusFlag::EventEnd;
}
