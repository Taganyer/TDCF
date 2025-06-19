//
// Created by taganyer on 25-5-24.
//

#include <tdcf/base/Errors.hpp>
#include <tdcf/cluster/Cluster.hpp>

using namespace tdcf;

void Cluster::start(unsigned cluster_size) {
    TDCF_CHECK_EXPR(_cluster_start == false)
    cluster_accept(cluster_size);
    if (_info.root_id) {
        Node::start(0);
    }
    cluster_start();
    _cluster_start = true;
}

StatusFlag Cluster::end() {
    if (!_cluster_start) return StatusFlag::Success;
    if (_start) {
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
    _cluster_start = false;
    return StatusFlag::Success;
}

StatusFlag Cluster::handle_message(CommunicatorEvent& event) {
    if (_start && event.id == _info.root_id) {
        return Node::handle_message(event);
    }
    if (!_cluster_start) return StatusFlag::ClusterOffline;
    auto& [type, id, meta, data] = event;
    StatusFlag flag = StatusFlag::Success;
    switch (type) {
        case CommunicatorEvent::ReceivedMessage:
            flag = handle_received_message(id, meta, data);
            break;
        case CommunicatorEvent::MessageSendable:
            flag = _info.send_delay_message(id);
            break;
        case CommunicatorEvent::DisconnectRequest:
            flag = handle_disconnect_request(id);
            break;
        default:
            TDCF_RAISE_ERROR("Recieved wrong event type");
    }
    return flag;
}
