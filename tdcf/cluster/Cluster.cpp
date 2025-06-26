//
// Created by taganyer on 25-5-24.
//

#include <tdcf/base/Errors.hpp>
#include <tdcf/cluster/Cluster.hpp>

using namespace tdcf;

void Cluster::start(unsigned cluster_size) {
    TDCF_CHECK_EXPR(_cluster_started == false)
    cluster_accept(cluster_size);
    if (_info.root_id()) {
        Node::start(0);
    }
    cluster_start();
    _info.set_cluster_size(cluster_size);
    _cluster_started = true;
}

StatusFlag Cluster::end_cluster() {
    if (!_cluster_started) return StatusFlag::Success;
    _cluster_closing = true;
    if (_node_agent_started) {
        _info.agent_factory = nullptr;
        assert(_info.progress_events.size() >= _cluster_events);
        while (_cluster_events) {
            StatusFlag flag = handle_a_loop();
            if (flag != StatusFlag::Success) return flag;
        }
    } else {
        assert(_info.progress_events.size() == _cluster_events);
        while (!_info.progress_events.empty()) {
            StatusFlag flag = handle_a_loop();
            if (flag != StatusFlag::Success) return flag;
        }
    }
    cluster_end();
    _info.set_cluster_size(0);
    _cluster_closing = false;
    _cluster_started = false;
    return StatusFlag::Success;
}

StatusFlag Cluster::handle_message(CommunicatorEvent& event) {
    if (_node_agent_started && event.id == _info.root_id()) {
        return Node::handle_message(event);
    }
    if (!_cluster_started) return StatusFlag::ClusterOffline;
    auto& [type, id, meta, data] = event;
    StatusFlag flag = StatusFlag::Success;
    switch (type) {
        case CommunicatorEvent::ReceivedMessage:
            flag = handle_received_message(id, meta, data);
            break;
        case CommunicatorEvent::MessageSendable:
            _info.send_delay_message(id);
            break;
        case CommunicatorEvent::DisconnectRequest:
            assert(_cluster_closing);
            flag = handle_disconnect_request(id);
            break;
        default:
            TDCF_RAISE_ERROR("Recieved wrong event type");
    }
    return flag;
}
