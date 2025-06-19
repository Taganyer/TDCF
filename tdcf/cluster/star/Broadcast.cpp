//
// Created by taganyer on 25-6-16.
//

#include <tdcf/base/Errors.hpp>
#include <tdcf/cluster/StarCluster.hpp>

using namespace tdcf;

StarCluster::Broadcast::Broadcast(ProgressType type, ProcessingRulesPtr rp) :
    EventProgress(type, std::move(rp)) {}

StatusFlag StarCluster::Broadcast::create(ProcessingRulesPtr rp, NodeInformation& info) {
    MetaData meta(info.progress_events_version++, OperationType::Broadcast);
    meta.progress_type = ProgressType::Root;
    auto [iter, success] = info.progress_events.emplace(
        meta, std::make_unique<Broadcast>(ProgressType::Root, std::move(rp)));
    TDCF_CHECK_EXPR(success)

    auto& self = static_cast<Broadcast&>(*iter->second);

    meta.stage = ClusterBroadcast::send_rule;
    for (auto& id : info.identity_list) {
        StatusFlag flag = info.send_message(id, meta, self.rule);
        TDCF_CHECK_SUCCESS(flag)
    }

    meta.stage = ClusterBroadcast::acquire_data;
    StatusFlag flag = info.acquire_data(iter, meta, self.rule);
    if (flag != StatusFlag::Success) {
        info.progress_events.erase(iter);
        return flag;
    }
    self._self = iter;
    return flag;
}

StatusFlag StarCluster::Broadcast::handle_event(const MetaData& meta,
                                                Variant& data, NodeInformation& info) {
    assert(meta.operation_type == OperationType::Broadcast);
    if (meta.stage == ClusterBroadcast::acquire_data) {
        if (_sent == 0) {
            _data = std::move(std::get<DataPtr>(data));
        }
        return send(info);
    }
    if (meta.stage == ClusterBroadcast::finish_ack) {
        ++_respond;
        assert(meta.progress_type == ProgressType::Node);
        if (_respond == info.identity_list.size()) {
            rule->finish_callback();
        }
        return StatusFlag::EventEnd;
    }
    TDCF_RAISE_ERROR(error type)
}

StatusFlag StarCluster::Broadcast::send(NodeInformation& info) {
    MetaData meta(_self->first);
    meta.stage = ClusterBroadcast::send_data;

    StatusFlag flag = StatusFlag::Success;
    for (; _sent < info.identity_list.size(); ++_sent) {
        meta.serial = _sent;
        auto& id = info.identity_list[_sent];
        flag = info.send_message(id, meta, _self->second->rule);
        if (flag != StatusFlag::Success) return flag;
        flag = info.send_message(id, meta, _data);
        if (flag != StatusFlag::Success) return flag;
    }
    if (_sent == info.identity_list.size()) _data = nullptr;
    return StatusFlag::Success;
}

StarCluster::BroadcastAgent::BroadcastAgent(ProcessingRulesPtr rp, ProgressEventsMI iter) :
    Broadcast(ProgressType::NodeRoot, std::move(rp)), _other(iter) {}

StatusFlag StarCluster::BroadcastAgent::create(ProcessingRulesPtr rp, ProgressEventsMI other,
                                               NodeInformation& info, EventProgressAgent **agent_ptr) {
    MetaData meta(info.progress_events_version++, OperationType::Broadcast);
    meta.progress_type = ProgressType::NodeRoot;
    auto [iter, success] = info.progress_events.emplace(
        meta, std::make_unique<BroadcastAgent>(std::move(rp), other));
    TDCF_CHECK_EXPR(success)

    auto& self = static_cast<BroadcastAgent&>(*iter->second);

    assert(meta.stage == AgentBroadcast::get_rule);
    meta.stage = ClusterBroadcast::send_rule;
    for (auto& id : info.identity_list) {
        StatusFlag flag = info.send_message(id, meta, self.rule);
        TDCF_CHECK_SUCCESS(flag)
    }

    *agent_ptr = &self;
    self._self = iter;
    return StatusFlag::Success;
}

StatusFlag StarCluster::BroadcastAgent::handle_event(const MetaData& meta, Variant& data,
                                                     NodeInformation& info) {
    assert(meta.operation_type == OperationType::Broadcast);
    if (meta.stage == AgentBroadcast::get_data) {
        if (_sent == 0) {
            _data = std::move(std::get<DataPtr>(data));
            info.store_data(rule, _data);
        }
        return send(info);
    }
    if (meta.stage == ClusterBroadcast::finish_ack) {
        ++_respond;
        if (_respond == info.identity_list.size()) {
            return close(info);
        }
        return StatusFlag::Success;
    }
    TDCF_RAISE_ERROR(error type)
}

StatusFlag StarCluster::BroadcastAgent::store(const MetaData& meta, Variant& data,
                                              NodeInformation& info) {
    return handle_event(meta, data, info);
}

StatusFlag StarCluster::BroadcastAgent::close(NodeInformation& info) const {
    MetaData meta(_other->first);
    meta.stage = AgentBroadcast::finish;
    info.processed_queue.emplace(_other, meta, SerializablePtr(nullptr));
    return StatusFlag::EventEnd;
}
