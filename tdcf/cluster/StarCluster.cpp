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
    ++_cluster_events; \
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
        while (_handle.communicator_events_size() && i < cluster_size) {
            auto [from_id, type, meta, data] = _handle.get_message();
            if (type == CommunicatorEvent::ConnectRequest) {
                _handle.accept(std::dynamic_pointer_cast<Identity>(data));
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
    for (uint32_t id = 0; id < _handle.cluster_size(); ++id) {
        if (id == _handle.root_serial()) continue;
        StatusFlag flag = _handle.send_message(0, id, meta, create_node_data());
        TDCF_CHECK_SUCCESS(flag)
    }
    _handle.agent_factory = std::make_unique<StarAgentFactory>();
}

void StarCluster::cluster_end() {
    MetaData meta;
    meta.operation_type = OperationType::Close;
    meta.data_type = SerializableBaseTypes::Null;
    meta.progress_type = ProgressType::Root;
    meta.stage = Star::close;
    StatusFlag flag = StatusFlag::Success;
    for (uint32_t id = 0; id < _handle.cluster_size(); ++id) {
        if (id == _handle.root_serial()) continue;
        flag = _handle.send_message(0, id, meta, nullptr);
        TDCF_CHECK_SUCCESS(flag)
        ++meta.serial;
    }
    while (!_handle.id_list_size()) {
        flag = handle_a_loop();
        TDCF_CHECK_SUCCESS(flag)
    }
}

SerializablePtr StarCluster::create_node_data() {
    return std::make_shared<StarAgent>();
}

StatusFlag StarCluster::handle_received_message(uint32_t from_id, const MetaData& meta,
                                                SerializablePtr& data) {
    auto iter = _handle.progress_events.find(meta);
    if (iter == _handle.progress_events.end()) return StatusFlag::Success;

    if (meta.operation_type == OperationType::Error) {
        iter->second->handle_error(_handle);
        return StatusFlag::EventEnd;
    }

    auto& [m, progress] = *iter;
    Variant variant(std::move(data));
    return progress->handle_event(meta, variant, _handle);
}

StatusFlag StarCluster::handle_disconnect_request(uint32_t from_id) {
    assert(from_id != _handle.root_serial());
    assert(!_handle.delayed_message(from_id));
    _handle.disconnect(from_id);
    return StatusFlag::Success;
}
