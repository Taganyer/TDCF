//
// Created by taganyer on 25-5-22.
//

#include <algorithm>
#include <tdcf/base/Errors.hpp>
#include <tdcf/cluster/StarCluster.hpp>
#include <tdcf/node/agents/star/StarAgent.hpp>

using namespace tdcf;


#define StarClusterFun(fun_name, class_name) \
StatusFlag StarCluster::fun_name(ProcessingRulesPtr rule_ptr) { \
    StatusFlag flag = class_name::create(std::move(rule_ptr), _handle); \
    if (flag != StatusFlag::Success) return flag; \
    return flag; \
}

#define StarAgentFactoryFun(type, class_name) \
StatusFlag StarCluster::StarAgentFactory::type(const ProcessingRulesPtr& rule, ProgressEventsMI iter, \
    Handle& handle, EventProgressAgent **agent_ptr) { \
    return class_name::create(rule, iter, handle, agent_ptr); \
}

#define StarFunAll(fun, cluster_class, agent_class) \
    StarClusterFun(fun, cluster_class) \
    StarAgentFactoryFun(fun, agent_class)


StarFunAll(broadcast, Broadcast, BroadcastAgent)

StarFunAll(scatter, Scatter, ScatterAgent)

StarFunAll(reduce, Reduce, ReduceAgent)

StarFunAll(all_reduce, AllReduce, AllReduceAgent)

StarFunAll(reduce_scatter, ReduceScatter, ReduceScatterAgent)

void StarCluster::cluster_accept(unsigned cluster_size) {
    for (unsigned i = 0; i < cluster_size;) {
        StatusFlag flag = active_communicator_events();
        TDCF_CHECK_EXPR(flag != StatusFlag::CommunicatorGetEventsError)
        Handle::MessageEvent event;
        while (i < cluster_size && _handle.get_message(event)) {
            auto& [type, from_id, meta, data] = event;
            if (type == CommunicatorEvent::ConnectRequest) {
                _handle.accept(from_id);
                auto [iter, success] = _handle.identities.emplace(from_id);
                assert(success);
                ++i;
            } else {
                TDCF_RAISE_ERROR(type == CommunicatorEvent::ConnectRequest ||
                    type == CommunicatorEvent::MessageSendable);
            }
        }
    }
}

void StarCluster::cluster_start() {
    MetaData meta;
    meta.operation_type = OperationType::AgentCreate;
    meta.stage = Star::start;
    meta.data1[0] = ClusterType::star;
    for (auto& id : _handle.identities) {
        if (_handle.root_identity() && id->equal_to(*_handle.root_identity())) continue;
        StatusFlag flag = _handle.send_message(id, meta, create_node_data());
        TDCF_CHECK_SUCCESS(flag)
    }
    _handle.agent_factory = std::make_unique<StarAgentFactory>();
}

void StarCluster::cluster_end() {
    MetaData meta;
    meta.operation_type = OperationType::Close;
    meta.stage = Star::close;
    StatusFlag flag = StatusFlag::Success;
    for (const auto& id : _handle.identities) {
        flag = _handle.send_message(id, meta, nullptr);
        TDCF_CHECK_SUCCESS(flag)
    }
    while (!_handle.identities.empty()) {
        flag = handle_a_loop();
        TDCF_CHECK_SUCCESS(flag)
    }
}

SerializablePtr StarCluster::create_node_data() {
    return std::make_shared<StarAgent>();
}

StatusFlag StarCluster::handle_received_message(const IdentityPtr& from_id, const MetaData& meta,
                                                Variant& variant) {
    auto iter = _handle.find_progress(meta.version);
    TDCF_CHECK_EXPR(_handle.check_progress(iter))
    auto& [m, progress] = *iter;
    return progress->handle_event(meta, variant, _handle);
}

StatusFlag StarCluster::handle_disconnect_request(const IdentityPtr& from_id) {
    assert(!_handle.root_identity() || !from_id->equal_to(*_handle.root_identity()));
    assert(!_handle.delayed_message(from_id));
    _handle.disconnect(from_id);
    _handle.identities.erase(from_id);
    return StatusFlag::Success;
}
