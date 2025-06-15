//
// Created by taganyer on 25-5-22.
//

#include <tdcf/base/Errors.hpp>
#include <tdcf/cluster/StarCluster.hpp>

using namespace tdcf;


StarCluster::StarCluster(IdentityPtr ip, CommunicatorPtr cp,
                         ProcessorPtr pp, IdentityPtr root_id,
                         unsigned cluster_size) :
    Cluster(std::move(ip), std::move(cp), std::move(pp), std::move(root_id)) {

}

StarCluster::~StarCluster() {
}

StatusFlag StarCluster::Broadcast::create(ProcessingRulesPtr rp, NodeInformation& info) {
    MetaData meta(info.progress_events_version++, MetaDataTypes::Broadcast);
    auto [iter, success] = info.progress_events.emplace(
        meta, std::make_unique<Broadcast>(std::move(rp)));
    TDCF_CHECK_EXPR(success)

    ++meta.stage;
    auto& self = static_cast<Broadcast&>(*iter->second);
    StatusFlag flag = info.acquire_data(iter, meta, self.rule);
    if (flag != StatusFlag::Success) {
        info.progress_events.erase(iter);
        return flag;
    }
    self._iter = iter;
    return flag;
}

StarCluster::Broadcast::Broadcast(ProcessingRulesPtr rp) :
    EventProgress(EventType::HTCBroadcast, std::move(rp)) {}

StatusFlag StarCluster::Broadcast::send(NodeInformation& node_info) {
    MetaData meta(_iter->first);
    meta.stage = 2;
    meta.ack_stage = 2;
    StatusFlag flag = StatusFlag::Success;
    for (; _sent < node_info.identity_list.size(); ++_sent) {
        meta.serial = _sent;
        meta.ack_serial = _sent;
        auto& id = node_info.identity_list[_sent];
        flag = node_info.send_message(id, meta, _iter->second->rule);
        if (flag != StatusFlag::Success) return flag;
        flag = node_info.send_message(id, meta, _data);
        if (flag != StatusFlag::Success) return flag;
    }
    if (_sent == node_info.identity_list.size()) _data = nullptr;
    return StatusFlag::Success;
}

StatusFlag StarCluster::Broadcast::handle_event(const MetaData& meta,
                                                Variant& data, NodeInformation& node_info) {
    assert(meta.type == MetaDataTypes::Broadcast);
    StatusFlag flag = StatusFlag::Success;
    if (meta.stage == 1) {
        if (_sent == 0) _data = std::move(std::get<DataPtr>(data));
        flag = send(node_info);
    } else if (meta.stage == 2) {
        ++_respond;
        if (_respond == node_info.identity_list.size()) {
            rule->finish_callback();
            flag = StatusFlag::EventEnd;
        }
    } else {
        TDCF_RAISE_ERROR(error type)
    }
    return flag;
}

StarCluster::Scatter::Scatter(ProcessingRulesPtr rp):
    EventProgress(EventType::HTCBroadcast, std::move(rp)) {}
