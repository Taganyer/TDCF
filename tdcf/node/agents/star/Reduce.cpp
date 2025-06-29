//
// Created by taganyer on 25-6-21.
//

#include <tdcf/base/Errors.hpp>
#include <tdcf/handle/Handle.hpp>
#include <tdcf/node/agents/star/StarAgent.hpp>

using namespace tdcf;

StarAgent::Reduce::Reduce(ProcessingRulesPtr rp, const MetaData& meta) :
    EventProgress(ProgressType::NodeRoot, std::move(rp)), _root_meta(meta) {}

StatusFlag StarAgent::Reduce::create(const MetaData& meta,
                                     ProcessingRulesPtr rp, Handle& info) {
    assert(meta.operation_type == OperationType::Reduce);
    assert(meta.stage == NodeAgentReduce::get_rule);

    MetaData new_meta(info.get_version(), OperationType::Reduce);
    new_meta.progress_type = ProgressType::Node;

    auto [iter, success] = info.progress_events.emplace(
        new_meta, std::make_unique<Reduce>(std::move(rp), meta));
    TDCF_CHECK_EXPR(success)

    auto& self = static_cast<Reduce&>(*iter->second);
    self._self = iter;

    if (!info.agent_factory) {
        new_meta.stage = NodeAgentReduce::acquire_data;
        info.acquire_data(iter, new_meta, self.rule);
        return StatusFlag::Success;
    }

    StatusFlag flag = info.agent_factory->reduce(self.rule, iter, info, &self._agent);
    if (flag != StatusFlag::Success || !self._agent) {
        info.progress_events.erase(iter);
        return flag;
    }
    return StatusFlag::Success;
}

StatusFlag StarAgent::Reduce::handle_event(const MetaData& meta,
                                           Variant& data, Handle& info) {
    assert(meta.operation_type == OperationType::Scatter);
    if (!_agent) {
        assert(meta.stage == NodeAgentReduce::acquire_data);
        return close(std::get<DataPtr>(data), info);
    }
    if (meta.stage == NodeAgentReduce::acquire_data) {
        return close(std::get<DataPtr>(data), info);
    }
    TDCF_RAISE_ERROR(meta.stage error type)
}

StatusFlag StarAgent::Reduce::close(DataPtr& data, Handle& info) const {
    MetaData meta(_root_meta);
    meta.stage = NodeAgentReduce::send_data;
    StatusFlag flag = info.send_message(info.root_identity(), meta, data);
    TDCF_CHECK_SUCCESS(flag)
    return StatusFlag::EventEnd;
}
