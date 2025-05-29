//
// Created by taganyer on 25-5-23.
//

#include <tdcf/base/Errors.hpp>
#include <tdcf/node/Node.hpp>

using namespace tdcf;

Node::Node(IdentityPtr idp, CommunicatorPtr cp, ProcessorPtr pp, InterpreterPtr inp) :
 _info(std::move(idp), std::move(cp), std::move(pp), std::move(inp)) {
    assert(_info.check());
}

StatusFlag Node::join_in_cluster(const IdentityPtr& cluster_id) {
    assert(!_node_data);
    SerializablePtr serializable_ptr;
    StatusFlag status_flag = _info.commander->connect_server(cluster_id, serializable_ptr);
    if (status_flag != StatusFlag::Success) return status_flag;
    _node_data = std::dynamic_pointer_cast<NodeAgent>(serializable_ptr);
    TDCF_CHECK_EXPR(_node_data)
    _node_data->cluster_id = cluster_id;
    return StatusFlag::Success;
}

StatusFlag Node::handle_a_loop() {
    assert(_node_data);
    StatusFlag flag = _info.commander->get_alive_event(_events);
    if (flag != StatusFlag::Success) return flag;
    for (auto& event : _events) {
        flag = _node_data->handle_node_event(_info, event);
        if (unlikely(flag == StatusFlag::ClusterOffline)) {
            TDCF_CHECK_SUCCESS(_info.commander->disconnect(_node_data->cluster_id))
            _node_data = nullptr;
            break;
        }
    }
    _events.clear();
    return flag;
}
