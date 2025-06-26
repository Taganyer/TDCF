//
// Created by taganyer on 25-6-22.
//

#include <tdcf/base/Errors.hpp>
#include <tdcf/detail/NodeInformation.hpp>
#include <tdcf/node/agents/star/StarAgent.hpp>

using namespace tdcf;

StarAgent::AllReduce::AllReduce(ProcessingRulesPtr rp, const MetaData& meta) :
    EventProgress(ProgressType::NodeRoot, std::move(rp)), _root_meta(meta) {}

StatusFlag StarAgent::AllReduce::create(const MetaData& meta,
                                        ProcessingRulesPtr rp, NodeInformation& info) {
    assert(meta.operation_type == OperationType::AllReduce);
    assert(meta.stage == NodeAgentAllReduce::get_rule);

    MetaData new_meta(info.get_version(), OperationType::AllReduce);
    new_meta.progress_type = ProgressType::Node;
    new_meta.serial = meta.serial;

    auto [iter, success] = info.progress_events.emplace(
        new_meta, std::make_unique<AllReduce>(std::move(rp), meta));
    TDCF_CHECK_EXPR(success)

    auto& self = static_cast<AllReduce&>(*iter->second);
    self._self = iter;

    if (!info.agent_factory) {
        new_meta.stage = NodeAgentAllReduce::acquire_data1;
        info.acquire_data(iter, new_meta, self.rule);
        return StatusFlag::Success;
    }

    StatusFlag flag = info.agent_factory->all_reduce(self.rule, iter, info, &self._agent);
    if (flag != StatusFlag::Success || !self._agent) {
        info.progress_events.erase(iter);
        return flag;
    }
    return StatusFlag::Success;
}

StatusFlag StarAgent::AllReduce::handle_event(const MetaData& meta,
                                              Variant& data, NodeInformation& info) {
    assert(meta.operation_type == OperationType::AllReduce);
    if (meta.stage == NodeAgentAllReduce::acquire_data1) {
        return acquire_data1(std::get<DataPtr>(data), info);
    }
    if (meta.stage == NodeAgentAllReduce::acquire_data2) {
        return acquire_data2(std::get<DataPtr>(data), info);
    }
    if (meta.stage == NodeAgentAllReduce::finish_ack) {
        return close(info);
    }
    TDCF_RAISE_ERROR(meta.stage error type)
}

StatusFlag StarAgent::AllReduce::acquire_data1(DataPtr& data, NodeInformation& info) const {
    MetaData meta(_root_meta);
    meta.stage = NodeAgentAllReduce::send_data1;
    StatusFlag flag = info.send_message(info.root_id(), meta, data);
    return flag;
}

StatusFlag StarAgent::AllReduce::acquire_data2(DataPtr& data, NodeInformation& info) const {
    if (!_agent) {
        info.store_data(rule, std::move(data));
        return close(info);
    }
    MetaData meta(_self->first);
    meta.stage = NodeAgentAllReduce::send_data2;
    Variant variant(std::move(data));
    StatusFlag flag = _agent->proxy_event(meta, variant, info);
    if (flag != StatusFlag::Success) {
        close(info);
        return flag;
    }
    return StatusFlag::Success;
}

StatusFlag StarAgent::AllReduce::close(NodeInformation& info) const {
    MetaData meta(_root_meta);
    meta.stage = NodeAgentAllReduce::finish;
    StatusFlag flag = info.send_message(info.root_id(), _root_meta, nullptr);
    TDCF_CHECK_SUCCESS(flag)
    return StatusFlag::EventEnd;
}
