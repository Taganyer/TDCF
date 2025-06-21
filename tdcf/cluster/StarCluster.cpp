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
    StatusFlag flag = class_name::create(std::move(rule_ptr), _info); \
    if (flag != StatusFlag::Success) return flag; \
    ++_cluster_events; \
    return flag; \
}

StarClusterFun(broadcast, Broadcast)

StarClusterFun(scatter, Scatter)

StarClusterFun(reduce, Reduce)

void StarCluster::cluster_accept(unsigned cluster_size) {
    for (unsigned i = 0; i < cluster_size;) {
        StatusFlag flag = active_communicator_events();
        TDCF_CHECK_EXPR(flag == StatusFlag::Success ||
            flag == StatusFlag::Timeout || flag == StatusFlag::FurtherWaiting)
        while (!_info.message_queue.empty() && i < cluster_size) {
            auto& [type, id, meta, data] = _info.message_queue.front();
            if (type == CommunicatorEvent::ConnectRequest) {
                flag = _info.communicator->accept(id);
                TDCF_CHECK_SUCCESS(flag)
                _info.identity_list.emplace_back(std::move(id));
                ++i;
            } else if (type == CommunicatorEvent::MessageSendable) {
                flag = _info.send_delay_message(id);
                TDCF_CHECK_SUCCESS(flag)
            } else {
                TDCF_RAISE_ERROR(type == CommunicatorEvent::ConnectRequest ||
                    type == CommunicatorEvent::MessageSendable);
            }
            _info.message_queue.pop();
        }
    }
    std::sort(_info.identity_list.begin(), _info.identity_list.end());
}

void StarCluster::cluster_start() {
    MetaData meta;
    meta.operation_type = OperationType::AgentCreate;
    meta.stage = Star::start;
    for (auto& id : _info.identity_list) {
        StatusFlag flag = _info.send_message(id, meta, create_node_data());
        ++meta.serial;
        TDCF_CHECK_SUCCESS(flag)
    }
    _info.agent_factory = std::make_unique<StarAgentFactory>();
}

void StarCluster::cluster_end() {
    MetaData meta;
    meta.operation_type = OperationType::Close;
    meta.data_type = SerializableBaseTypes::Null;
    meta.progress_type = ProgressType::Root;
    meta.stage = Star::close;
    StatusFlag flag = StatusFlag::Success;
    for (auto& id : _info.identity_list) {
        flag = _info.send_message(id, meta, nullptr);
        TDCF_CHECK_SUCCESS(flag)
        ++meta.serial;
    }
    while (!_info.identity_list.empty()) {
        flag = handle_a_loop();
        TDCF_CHECK_SUCCESS(flag)
    }
}

SerializablePtr StarCluster::create_node_data() {
    return std::make_shared<StarAgent>();
}

StatusFlag StarCluster::handle_received_message(IdentityPtr& id, const MetaData& meta,
                                                SerializablePtr& data) {
    auto iter = _info.progress_events.find(meta);
    assert(iter != _info.progress_events.end());
    auto& [m, progress] = *iter;
    Variant variant(std::move(data));
    return progress->handle_event(meta, variant, _info);
}

StatusFlag StarCluster::handle_disconnect_request(IdentityPtr& id) {
    assert(id != _info.root_id);
    assert(id);
    assert(!_info.delayed_message(id));
    StatusFlag flag = _info.communicator->disconnect(id);
    TDCF_CHECK_SUCCESS(flag)
    _info.identity_list.erase(std::lower_bound(_info.identity_list.begin(), _info.identity_list.end(), id));
    return StatusFlag::Success;
}

#define StarAgentFactoryFun(type, class_name) \
StatusFlag StarCluster::StarAgentFactory::type(const ProcessingRulesPtr& rule, ProgressEventsMI iter, \
                                               NodeInformation& info, EventProgressAgent **agent_ptr) { \
    return class_name::create(rule, iter, info, agent_ptr); \
}

StarAgentFactoryFun(broadcast, BroadcastAgent)

StarAgentFactoryFun(scatter, ScatterAgent)
