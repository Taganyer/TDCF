//
// Created by taganyer on 25-5-23.
//

#include <tdcf/base/Errors.hpp>
#include <tdcf/node/Node.hpp>

using namespace tdcf;

Node::Node(IdentityPtr idp, TransmitterPtr tp, CommanderPtr cp, ProcessorPtr pp, InterpreterPtr inp) :
 _info(std::move(idp), std::move(tp), std::move(cp), std::move(pp), std::move(inp)) {
    assert(_info.check());
}

StatusFlag Node::join_in_cluster(const IdentityPtr& cluster_id) {
    assert(!_node_data);
    SerializablePtr serializable_ptr;
    StatusFlag status_flag = _info.commander->connect_server(cluster_id, serializable_ptr);
    if (status_flag != StatusFlag::Success) return status_flag;
    IdentityPtr identity_ptr;
    status_flag = _info.transmitter->connect_server(cluster_id);
    if (status_flag != StatusFlag::Success) {
        TDCF_CHECK_SUCCESS(_info.commander->disconnect(cluster_id))
        return status_flag;
    }
    _node_data = std::dynamic_pointer_cast<NodeAgent>(serializable_ptr);
    if (!_node_data) return StatusFlag::ErrorType;
    _node_data->cluster_id = cluster_id;
    return StatusFlag::Success;
}

StatusFlag Node::handle_a_loop() {
    assert(_node_data);
    StatusFlag flag = _node_data->handle_node_loop(_info);
    if (flag == StatusFlag::ClusterOffline) {
        TDCF_CHECK_SUCCESS(_info.commander->disconnect(_node_data->cluster_id))
        TDCF_CHECK_SUCCESS(_info.transmitter->disconnect(_node_data->cluster_id))
        _node_data = nullptr;
    }
    return flag;
}
