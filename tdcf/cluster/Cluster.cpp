//
// Created by taganyer on 25-5-24.
//

#include <tdcf/base/Errors.hpp>
#include <tdcf/cluster/Cluster.hpp>
#include <tdcf/handle/CommunicatorHandle.hpp>

using namespace tdcf;

void Cluster::start(unsigned cluster_size) {
    TDCF_CHECK_EXPR(_cluster_started == false)
    cluster_accept(cluster_size);
    if (_handle.root_identity()) {
        Node::start(0);
    }
    cluster_start();
    _handle.set_cluster_size(cluster_size);
    _cluster_started = true;
}

StatusFlag Cluster::end_cluster() {
    if (!_cluster_started) return StatusFlag::Success;
    _cluster_closing = true;
    if (_node_agent_started) {
        _handle.agent_factory = nullptr;
        assert(_handle.total_events() >= _handle.cluster_events());
        while (_handle.cluster_events()) {
            StatusFlag flag = handle_a_loop();
            if (flag != StatusFlag::Success) return flag;
        }
    } else {
        assert(_handle.total_events() == _handle.cluster_events());
        while (_handle.total_events()) {
            StatusFlag flag = handle_a_loop();
            if (flag != StatusFlag::Success) return flag;
        }
    }
    cluster_end();
    _handle.set_cluster_size(0);
    _cluster_closing = false;
    _cluster_started = false;
    return StatusFlag::Success;
}

StatusFlag Cluster::handle_message(Handle::MessageEvent& event) {
    if (_node_agent_started && event.id->equal_to(*_handle.root_identity())) {
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
