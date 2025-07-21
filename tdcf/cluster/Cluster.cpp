//
// Created by taganyer on 25-5-24.
//

#include <tdcf/base/Errors.hpp>
#include <tdcf/cluster/Cluster.hpp>
#include <tdcf/handle/CommunicatorHandle.hpp>

using namespace tdcf;

Cluster::Cluster(IdentityPtr ip, CommunicatorPtr cp, ProcessorPtr pp) :
    Node(std::move(ip), std::move(cp), std::move(pp)) {}

Cluster::~Cluster() { assert(!_cluster_started); }

void Cluster::start_cluster(const IdentitySet& child_nodes, bool as_child_node) {
    TDCF_CHECK_EXPR(_cluster_started == false)
    _cluster_staring = true;
    cluster_connect_children(child_nodes);
    if (as_child_node && !_node_agent_started) {
        start_node();
    }
    cluster_start();
    _cluster_staring = false;
    _cluster_started = true;
}

StatusFlag Cluster::end_cluster() {
    if (!_cluster_started) return StatusFlag::Success;
    _cluster_closing = true;
    _handle.agent_factory = nullptr;
    assert(node_agent_started() && _handle.total_events() >= _handle.cluster_events()
        || _handle.total_events() == _handle.cluster_events());
    while (_handle.cluster_events()) {
        StatusFlag flag = handle_a_loop();
        if (flag != StatusFlag::Success) return flag;
    }
    cluster_end();
    _handle.destroy_cluster_data();
    _cluster_closing = false;
    _cluster_started = false;
    return StatusFlag::Success;
}

StatusFlag Cluster::handle_received_message(const IdentityPtr& from_id, const MetaData& meta,
                                                Variant& variant) {
    auto iter = _handle.find_progress(meta.version);
    TDCF_CHECK_EXPR(_handle.check_progress(iter))
    auto& [m, progress] = *iter;
    return progress->handle_event(meta, variant, _handle);
}

StatusFlag Cluster::handle_message(Handle::MessageEvent& event) {
    if (_node_agent_started && !from_sub_cluster(event.id)) {
        return Node::handle_message(event);
    }
    if (!_cluster_started) return StatusFlag::ClusterOffline;
    auto& [type, from_id, meta, variant] = event;
    assert(meta.operation_type != OperationType::Close);
    StatusFlag flag = StatusFlag::Success;
    switch (type) {
        case CommunicatorEvent::ReceivedMessage:
            flag = handle_received_message(from_id, meta, variant);
            break;
        case CommunicatorEvent::MessageSendable:
            flag = _handle.send_delay_message(from_id);
            break;
        case CommunicatorEvent::DisconnectRequest:
            assert(_cluster_closing);
            flag = handle_disconnect_request(from_id);
            break;
        default:
            TDCF_RAISE_ERROR("Recieved wrong event type");
    }
    return flag;
}
