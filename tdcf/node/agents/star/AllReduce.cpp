//
// Created by taganyer on 25-6-22.
//

#include <tdcf/base/Errors.hpp>
#include <tdcf/base/types/Star.hpp>
#include <tdcf/handle/Handle.hpp>
#include <tdcf/node/agents/star/StarAgent.hpp>

using namespace tdcf;

using namespace tdcf::star;

StarAgent::AllReduce::AllReduce(uint32_t version, ProcessingRulesPtr rp) :
    EventProgress(OperationType::AllReduce, ProgressType::NodeRoot, version, std::move(rp)) {}

StatusFlag StarAgent::AllReduce::create(uint32_t version, const MetaData& meta,
                                        ProcessingRulesPtr rp, Handle& handle) {
    assert(meta.operation_type == OperationType::AllReduce);
    assert(meta.stage == N_AllReduce::get_rule);

    auto iter = handle.create_progress(std::make_unique<AllReduce>(version, std::move(rp)));

    auto& self = static_cast<AllReduce&>(*iter->second);
    self.serial = meta.serial;

    MetaData new_meta = self.create_meta();
    if (!handle.agent_factory) {
        new_meta.stage = Public_AllReduce::node_acquire;
        handle.acquire_data(iter, new_meta, self.rule);
        return StatusFlag::Success;
    }

    StatusFlag flag = handle.agent_factory->all_reduce(self.rule, iter, handle, &self._agent);
    if (flag != StatusFlag::Success || !self._agent) {
        handle.destroy_progress(iter);
        return flag;
    }
    return StatusFlag::Success;
}

StatusFlag StarAgent::AllReduce::handle_event(const MetaData& meta,
                                              Variant& data, Handle& handle) {
    assert(meta.operation_type == OperationType::AllReduce);
    if (meta.stage == Public_AllReduce::node_acquire) {
        return acquire_data1(std::get<DataPtr>(data), handle);
    }
    if (meta.stage == N_AllReduce::acquire_data2) {
        return acquire_data2(std::get<DataPtr>(data), handle);
    }
    if (meta.stage == Public_AllReduce::node_finish_ack) {
        return close(handle);
    }
    TDCF_RAISE_ERROR(meta.stage error type)
}

StatusFlag StarAgent::AllReduce::acquire_data1(DataPtr& data, Handle& handle) const {
    MetaData meta = create_meta();
    meta.stage = N_AllReduce::send_data1;
    StatusFlag flag = handle.send_progress_message(version, handle.agent_data<IdentityPtr>(), meta, data);
    TDCF_CHECK_SUCCESS(flag)
    return StatusFlag::Success;
}

StatusFlag StarAgent::AllReduce::acquire_data2(DataPtr& data, Handle& handle) const {
    if (!_agent) {
        handle.store_data(rule, data);
        return close(handle);
    }
    MetaData meta = create_meta();
    meta.stage = Public_AllReduce::node_store;
    Variant variant(std::move(data));
    StatusFlag flag = _agent->proxy_event(meta, variant, handle);
    TDCF_CHECK_SUCCESS(flag)
    return StatusFlag::Success;
}

StatusFlag StarAgent::AllReduce::close(Handle& handle) const {
    MetaData meta = create_meta();
    meta.stage = N_AllReduce::finish;
    StatusFlag flag = handle.send_progress_message(version, handle.agent_data<IdentityPtr>(), meta, nullptr);
    TDCF_CHECK_SUCCESS(flag)
    return StatusFlag::EventEnd;
}
