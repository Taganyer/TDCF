//
// Created by taganyer on 25-6-26.
//

#include <tdcf/base/Errors.hpp>
#include <tdcf/handle/Handle.hpp>
#include <tdcf/node/agents/star/StarAgent.hpp>

using namespace tdcf;

StarAgent::ReduceScatter::ReduceScatter(uint32_t version, ProcessingRulesPtr rp) :
    EventProgress(OperationType::ReduceScatter, ProgressType::NodeRoot, version, std::move(rp)) {}

StatusFlag StarAgent::ReduceScatter::create(uint32_t version, const MetaData& meta,
                                            ProcessingRulesPtr rp, Handle& handle) {
    assert(meta.operation_type == OperationType::ReduceScatter);
    assert(meta.stage == NodeAgentReduceScatter::get_rule);

    auto iter = handle.create_progress(std::make_unique<ReduceScatter>(version, std::move(rp)));

    auto& self = static_cast<ReduceScatter&>(*iter->second);
    self.serial = meta.serial;

    MetaData new_meta = self.create_meta();
    if (!handle.agent_factory) {
        new_meta.stage = NodeAgentReduceScatter::acquire_data1;
        handle.acquire_data(iter, new_meta, self.rule);
        return StatusFlag::Success;
    }

    StatusFlag flag = handle.agent_factory->reduce_scatter(self.rule, iter, handle, &self._agent);
    if (flag != StatusFlag::Success || !self._agent) {
        handle.destroy_progress(iter);
        return flag;
    }
    return StatusFlag::Success;
}

StatusFlag StarAgent::ReduceScatter::handle_event(const MetaData& meta,
                                                  Variant& data, Handle& handle) {
    assert(meta.operation_type == OperationType::ReduceScatter);
    if (meta.stage == NodeAgentReduceScatter::acquire_data1) {
        return acquire_data1(std::get<DataPtr>(data), handle);
    }
    if (meta.stage == NodeAgentReduceScatter::acquire_data2) {
        return acquire_data2(std::get<DataPtr>(data), handle);
    }
    if (meta.stage == NodeAgentReduceScatter::finish_ack) {
        return close(handle);
    }
    TDCF_RAISE_ERROR(meta.stage error type)
}

StatusFlag StarAgent::ReduceScatter::acquire_data1(DataPtr& data, Handle& handle) const {
    MetaData meta = create_meta();
    meta.stage = NodeAgentReduceScatter::send_data1;
    StatusFlag flag = handle.send_progress_message(version, handle.agent_data<IdentityPtr>(),
                                                   meta, std::move(data));
    return flag;
}

StatusFlag StarAgent::ReduceScatter::acquire_data2(DataPtr& data, Handle& handle) const {
    if (!_agent) {
        handle.store_data(rule, data);
        return close(handle);
    }
    MetaData meta = create_meta();
    meta.stage = NodeAgentReduceScatter::send_data2;
    Variant variant = std::move(data);
    StatusFlag flag = _agent->proxy_event(meta, variant, handle);
    if (flag != StatusFlag::Success) {
        close(handle);
        return flag;
    }
    return StatusFlag::Success;
}

StatusFlag StarAgent::ReduceScatter::close(Handle& handle) const {
    MetaData meta = create_meta();
    meta.stage = NodeAgentReduceScatter::finish;
    StatusFlag flag = handle.send_progress_message(version, handle.agent_data<IdentityPtr>(), meta, nullptr);
    TDCF_CHECK_SUCCESS(flag)
    return StatusFlag::EventEnd;
}
