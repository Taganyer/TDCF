//
// Created by taganyer on 25-6-20.
//

#include <tdcf/base/Errors.hpp>
#include <tdcf/cluster/StarCluster.hpp>

using namespace tdcf;

StarCluster::Scatter::Scatter(ProgressType type, ProcessingRulesPtr rp) :
    EventProgress(type, std::move(rp)) {}

StatusFlag StarCluster::Scatter::create(ProcessingRulesPtr rp, NodeInformation& info) {
    MetaData meta(info.progress_events_version++, OperationType::Scatter);
    meta.progress_type = ProgressType::Root;

    auto [iter, success] = info.progress_events.emplace(
        meta, std::make_unique<Scatter>(ProgressType::Root, std::move(rp)));
    TDCF_CHECK_EXPR(success);

    auto& self = static_cast<Scatter&>(*iter->second);
    self._self = iter;

    meta.stage = ClusterScatter::acquire_data;
    StatusFlag flag = info.acquire_data(iter, meta, self.rule);
    if (flag != StatusFlag::Success) {
        info.progress_events.erase(iter);
        return flag;
    }

    meta.stage = ClusterScatter::send_rule;
    for (auto& id : info.identity_list) {
        flag = info.send_message(id, meta, self.rule);
        TDCF_CHECK_SUCCESS(flag)
    }
    return flag;
}

StatusFlag StarCluster::Scatter::handle_event(const MetaData& meta,
                                              Variant& data, NodeInformation& info) {
    assert(meta.operation_type == OperationType::Scatter);
    if (meta.stage == ClusterScatter::acquire_data) {
        assert(_sent == 0);
        return scatter_data(std::get<DataPtr>(data), info);
    }
    if (meta.stage == ClusterScatter::scatter_data) {
        assert(_sent == 0);
        return send_data(0, std::get<DataSet>(data), info);
    }
    if (meta.stage == ClusterScatter::finish_ack) {
        assert(meta.progress_type == ProgressType::Node);
        ++_respond;
        if (_respond == info.cluster_size) {
            rule->finish_callback();
        }
        return StatusFlag::EventEnd;
    }
    TDCF_RAISE_ERROR(meta.stage error type)
}

StatusFlag StarCluster::Scatter::scatter_data(DataPtr& data, NodeInformation& info) const {
    MetaData meta(_self->first);
    meta.stage = ClusterScatter::scatter_data;
    return info.scatter_data(_self, meta, rule, info.cluster_size, data);
}

StatusFlag StarCluster::Scatter::send_data(unsigned offset,
                                           DataSet& set, NodeInformation& info) {
    TDCF_CHECK_EXPR(set.size() == info.cluster_size);
    MetaData meta(_self->first);
    meta.stage = ClusterScatter::send_data;

    assert(info.cluster_size == info.identity_list.size());
    for (; _sent < info.cluster_size; ++_sent) {
        meta.serial = _sent;
        auto& id = info.identity_list[_sent];
        StatusFlag flag = info.send_message(id, meta, set[_sent + offset]);
        TDCF_CHECK_SUCCESS(flag)
    }
    return StatusFlag::Success;
}

StarCluster::ScatterAgent::ScatterAgent(ProcessingRulesPtr rp, ProgressEventsMI iter) :
    Scatter(ProgressType::NodeRoot, std::move(rp)), _other(iter) {}

StatusFlag StarCluster::ScatterAgent::create(ProcessingRulesPtr rp, ProgressEventsMI other,
                                             NodeInformation& info, EventProgressAgent **agent_ptr) {
    MetaData meta(info.progress_events_version++, OperationType::Scatter);
    meta.progress_type = ProgressType::NodeRoot;
    auto [iter, success] = info.progress_events.emplace(
        meta, std::make_unique<ScatterAgent>(std::move(rp), other));
    TDCF_CHECK_EXPR(success);

    auto& self = static_cast<ScatterAgent&>(*iter->second);
    *agent_ptr = &self;
    self._self = iter;

    meta.stage = AgentScatter::send_rule;
    for (auto& id : info.identity_list) {
        StatusFlag flag = info.send_message(id, meta, self.rule);
        TDCF_CHECK_SUCCESS(flag)
    }

    return StatusFlag::Success;
}

StatusFlag StarCluster::ScatterAgent::handle_event(const MetaData& meta,
                                                   Variant& data, NodeInformation& info) {
    assert(meta.operation_type == OperationType::Scatter);
    if (meta.stage == AgentScatter::get_data) {
        assert(_sent == 0);
        auto& set = std::get<DataSet>(data);
        info.store_data(rule, set.front());
        return send_data(1, set, info);
    }
    if (meta.stage == AgentScatter::finish_ack) {
        ++_respond;
        if (_respond == info.cluster_size) {
            return close(info);
        }
        return StatusFlag::Success;
    }
    TDCF_RAISE_ERROR(meta.stage error type)
}

StatusFlag StarCluster::ScatterAgent::proxy_event(const MetaData& meta,
                                            Variant& data, NodeInformation& info) {
    return handle_event(meta, data, info);
}

StatusFlag StarCluster::ScatterAgent::close(NodeInformation& info) const {
    MetaData meta(_self->first);
    meta.stage = AgentScatter::finish;
    info.processed_queue.emplace(_other, meta, SerializablePtr());
    return StatusFlag::EventEnd;
}
