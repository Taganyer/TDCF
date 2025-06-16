//
// Created by taganyer on 25-6-16.
//

#include <tdcf/base/Errors.hpp>
#include <tdcf/detail/OperationInterpreter.hpp>
#include <tdcf/cluster/StarCluster.hpp>

using namespace tdcf;

StatusFlag StarCluster::Broadcast::create(ProcessingRulesPtr rp, NodeInformation& info) {
    MetaData meta(info.progress_events_version++, MetaDataTypes::Broadcast);
    auto [iter, success] = info.progress_events.emplace(
        meta, std::make_unique<Broadcast>(EventType::HTCBroadcast, std::move(rp)));
    TDCF_CHECK_EXPR(success)

    meta.stage = ClusterBroadcast::acquire;
    auto& self = static_cast<Broadcast&>(*iter->second);
    StatusFlag flag = info.acquire_data(iter, meta, self.rule);
    if (flag != StatusFlag::Success) {
        info.progress_events.erase(iter);
        return flag;
    }
    self._self = iter;
    return flag;
}

StatusFlag StarCluster::Broadcast::handle_event(const MetaData& meta,
                                                Variant *data, NodeInformation& node_info) {
    assert(meta.type == MetaDataTypes::Broadcast);
    if (meta.stage == ClusterBroadcast::acquire) {
        if (_sent == 0) {
            assert(data);
            _data = std::move(std::get<DataPtr>(*data));
        }
        return send(node_info);
    }
    if (meta.stage == ClusterBroadcast::finish_ack) {
        ++_respond;
        if (_respond == node_info.identity_list.size()) {
            rule->finish_callback();
        }
        return StatusFlag::EventEnd;
    }
    TDCF_RAISE_ERROR(error type)
}

StarCluster::Broadcast::Broadcast(EventType type, ProcessingRulesPtr rp) :
    EventProgress(type, std::move(rp)) {}

StatusFlag StarCluster::Broadcast::send(NodeInformation& node_info) {
    MetaData meta(_self->first);
    meta.stage = ClusterBroadcast::send;

    StatusFlag flag = StatusFlag::Success;
    for (; _sent < node_info.identity_list.size(); ++_sent) {
        meta.serial = _sent;
        meta.ack_serial = _sent;
        auto& id = node_info.identity_list[_sent];
        flag = node_info.send_message(id, meta, _self->second->rule);
        if (flag != StatusFlag::Success) return flag;
        flag = node_info.send_message(id, meta, _data);
        if (flag != StatusFlag::Success) return flag;
    }
    if (_sent == node_info.identity_list.size()) _data = nullptr;
    return StatusFlag::Success;
}

StatusFlag StarCluster::BroadcastAgent::create(ProcessingRulesPtr rp, ProgressEventsMI other,
                                               NodeInformation& info, EventProgressAgent **agent_ptr) {
    MetaData meta(info.progress_events_version++, MetaDataTypes::Broadcast);
    auto [iter, success] = info.progress_events.emplace(
        meta, std::make_unique<BroadcastAgent>(std::move(rp), other));
    TDCF_CHECK_EXPR(success)

    auto& self = static_cast<BroadcastAgent&>(*iter->second);
    *agent_ptr = &self;
    self._self = iter;
    return StatusFlag::Success;
}

StatusFlag StarCluster::BroadcastAgent::handle_event(const MetaData& meta, Variant *data,
                                                     NodeInformation& node_info) {
    assert(meta.type == MetaDataTypes::Broadcast);
    if (meta.stage == AgentBroadcast::acquire) {
        if (_sent == 0) {
            assert(data);
            _data = std::move(std::get<DataPtr>(*data));
        }
        return send(node_info);
    }
    if (meta.stage == ClusterBroadcast::finish_ack) {
        ++_respond;
        if (_respond == node_info.identity_list.size()) {
            return close(node_info);
        }
        return StatusFlag::Success;
    }
    TDCF_RAISE_ERROR(error type)
}

StatusFlag StarCluster::BroadcastAgent::store(const MetaData& meta, Variant *data,
                                              NodeInformation& node_info) {
    return handle_event(meta, data, node_info);
}

StatusFlag StarCluster::BroadcastAgent::close(NodeInformation& node_info) const {
    MetaData meta(0, MetaDataTypes::Broadcast);
    meta.stage = AgentBroadcast::finish;

    StatusFlag flag = _other->second->handle_event(meta, nullptr, node_info);
    if (flag == StatusFlag::EventEnd) {
        node_info.progress_events.erase(_other);
        return StatusFlag::EventEnd;
    }
    assert(flag != StatusFlag::Success);
    return flag;
}

StarCluster::BroadcastAgent::BroadcastAgent(ProcessingRulesPtr rp, ProgressEventsMI iter) :
    Broadcast(EventType::CTCBroadcast, std::move(rp)), _other(iter) {}

