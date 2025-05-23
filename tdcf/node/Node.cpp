//
// Created by taganyer on 25-5-23.
//

#include "Node.hpp"

#include "../error/error.hpp"

using namespace tdcf;

StatusFlag Node::join_in_cluster(const IdentityPtr& cluster_id) {
    assert(!_node_data);
    SerializablePtr serializable_ptr;
    StatusFlag status_flag = _commander->connect_server(cluster_id, serializable_ptr);
    if (status_flag != StatusFlag::Success) return status_flag;
    IdentityPtr identity_ptr;
    status_flag = _transmitter->connect_server(cluster_id);
    if ((status_flag != StatusFlag::Success)) {
        TDCF_CHECK_SUCCESS(_commander->disconnect(cluster_id))
        return status_flag;
    }
    _node_data = std::dynamic_pointer_cast<NodeData>(serializable_ptr);
    if (!_node_data) return StatusFlag::ErrorType;
    _cluster_id = cluster_id;
    _node_data->_id = _id.get();
    _node_data->_commander = _commander.get();
    _node_data->_transmitter = _transmitter.get();
    _node_data->_processor = _processor.get();
    return StatusFlag::Success;
}

StatusFlag Node::handle_a_loop() {
    assert(_node_data && _cluster_id);
    StatusFlag flag = _node_data->handle_a_loop();
    if (flag == StatusFlag::ClusterOffline) {
        TDCF_CHECK_SUCCESS(_node_data->_commander->disconnect(_cluster_id))
        TDCF_CHECK_SUCCESS(_node_data->_transmitter->disconnect(_cluster_id))
        _node_data = nullptr;
        _cluster_id = nullptr;
    }
    return flag;
}
