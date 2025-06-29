//
// Created by taganyer on 25-6-26.
//

#include <tdcf/base/Errors.hpp>
#include <tdcf/handle/Handle.hpp>
#include <tdcf/node/agents/star/StarAgent.hpp>

using namespace tdcf;

StarAgent::ReduceScatter::ReduceScatter(ProcessingRulesPtr rp, const MetaData& meta) :
    EventProgress(ProgressType::NodeRoot, std::move(rp)), _root_meta(meta) {}

StatusFlag StarAgent::ReduceScatter::create(const MetaData& meta,
                                            ProcessingRulesPtr rp, Handle& info) {
    assert(meta.operation_type == OperationType::ReduceScatter);
    assert(meta.stage == NodeAgentReduceScatter::get_rule);

    MetaData new_meta(info.get_version(), OperationType::ReduceScatter);
    new_meta.progress_type = ProgressType::Node;
    new_meta.serial = meta.serial;

    auto [iter, success] = info.progress_events.emplace(
        new_meta, std::make_unique<ReduceScatter>(std::move(rp), meta));

    auto& self = static_cast<ReduceScatter&>(*iter->second);
    self._self = iter;

    if (!info.agent_factory) {
        new_meta.stage = NodeAgentReduceScatter::acquire_data1;
        info.acquire_data(iter, new_meta, self.rule);
        return StatusFlag::Success;
    }

    StatusFlag flag = info.agent_factory->reduce_scatter(self.rule, iter, info, &self._agent);
    if (flag != StatusFlag::Success || !self._agent) {
        info.progress_events.erase(iter);
        return flag;
    }
    return StatusFlag::Success;
}

StatusFlag StarAgent::ReduceScatter::handle_event(const MetaData& meta,
                                                  Variant& data, Handle& info) {
    assert(meta.operation_type == OperationType::ReduceScatter);
    if (meta.stage == NodeAgentReduceScatter::acquire_data1) {
        return acquire_data1(std::get<DataPtr>(data), info);
    }
    if (meta.stage == NodeAgentReduceScatter::acquire_data2) {
        return acquire_data2(std::get<DataPtr>(data), info);
    }
    if (meta.stage == NodeAgentReduceScatter::finish_ack) {
        return close(info);
    }
    TDCF_RAISE_ERROR(meta.stage error type)
}

StatusFlag StarAgent::ReduceScatter::acquire_data1(DataPtr& data, Handle& info) const {
    MetaData meta(_root_meta);
    meta.stage = NodeAgentReduceScatter::send_data1;
    StatusFlag flag = info.send_message(info.root_identity(), meta, data);
    return flag;
}

StatusFlag StarAgent::ReduceScatter::acquire_data2(DataPtr& data, Handle& info) const {
    if (!_agent) {
        info.store_data(rule, data);
        return close(info);
    }
    MetaData meta(_self->first);
    meta.stage = NodeAgentReduceScatter::send_data2;
    Variant variant = std::move(data);
    StatusFlag flag = _agent->proxy_event(meta, variant, info);
    if (flag != StatusFlag::Success) {
        close(info);
        return flag;
    }
    return StatusFlag::Success;
}

StatusFlag StarAgent::ReduceScatter::close(Handle& info) const {
    MetaData meta(_root_meta);
    meta.stage = NodeAgentReduceScatter::finish;
    StatusFlag flag = info.send_message(info.root_identity(), _root_meta, nullptr);
    TDCF_CHECK_SUCCESS(flag)
    return StatusFlag::EventEnd;
}
