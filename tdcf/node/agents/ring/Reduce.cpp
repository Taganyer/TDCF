//
// Created by taganyer on 25-7-8.
//

#include <tdcf/base/Errors.hpp>
#include <tdcf/base/types/Ring.hpp>
#include <tdcf/node/agents/ring/RingAgent.hpp>

using namespace tdcf;

using namespace tdcf::ring;

RingAgent::Reduce::Reduce(uint32_t version, ProcessingRulesPtr rp) :
    EventProgress(OperationType::Reduce, ProgressType::Node, version, std::move(rp)) {}

StatusFlag RingAgent::Reduce::create(uint32_t version, const MetaData& meta,
                                     ProcessingRulesPtr rp, Handle& handle) {
    assert(meta.operation_type == OperationType::Reduce);
    assert(meta.stage == N_Reduce::get_rule);

    auto& [send, receive, serial] = handle.agent_data<RingAgentData>();

    auto iter = handle.create_progress(std::make_unique<Reduce>(version, std::move(rp)));

    auto& self = static_cast<Reduce&>(*iter->second);
    self._self = iter;

    MetaData new_meta = self.create_meta();
    if (serial != 1) {
        new_meta.stage = N_Reduce::send_rule;
        StatusFlag flag = StatusFlag::Success;
        flag = handle.send_progress_message(version, send, new_meta, self.rule);
        if (flag != StatusFlag::Success) {
            handle.destroy_progress(iter);
            return flag;
        }
    }

    if (!handle.agent_factory) {
        new_meta.stage = Public_Reduce::node_acquire;
        handle.acquire_data(iter, new_meta, self.rule);
        return StatusFlag::Success;
    }

    StatusFlag flag = handle.agent_factory->reduce(self.rule, iter, handle, &self._agent);
    if (flag != StatusFlag::Success || !self._agent) {
        handle.destroy_progress(iter);
        return flag;
    }

    return StatusFlag::Success;
}

StatusFlag RingAgent::Reduce::handle_event(const MetaData& meta,
                                           Variant& data, Handle& handle) {
    assert(meta.operation_type == OperationType::Reduce);
    if (meta.stage == Public_Reduce::node_acquire) {
        auto& set = std::get<DataSet>(data);
        uint32_t rest_size = set.size();
        for (auto& d : set) {
            --rest_size;
            acquire_data(d, rest_size, handle);
        }
        return StatusFlag::Success;
    }
    if (meta.stage == N_Reduce::get_data) {
        return acquire_data(std::get<DataPtr>(data), meta.rest_data, handle);
    }
    if (meta.stage == N_Reduce::reduce_data) {
        return close(std::get<DataSet>(data), handle);
    }
    TDCF_RAISE_ERROR(meta.stage error type)
}

StatusFlag RingAgent::Reduce::acquire_data(DataPtr& data,
                                           uint32_t rest_size, Handle& handle) {
    _set.emplace_back(std::move(data));
    if (rest_size == 0) ++_step;
    if (_step == 2) {
        MetaData meta = create_meta();
        meta.stage = N_Reduce::reduce_data;
        handle.reduce_data(_self, meta, rule, std::move(_set));
    }
    return StatusFlag::Success;
}

StatusFlag RingAgent::Reduce::close(DataSet& dataset, Handle& handle) const {
    auto& [send, receive, serial] = handle.agent_data<RingAgentData>();
    MetaData meta = create_meta();
    meta.stage = N_Reduce::send_data;
    meta.rest_data = dataset.size();

    for (auto& data : dataset) {
        --meta.rest_data;
        StatusFlag flag = handle.send_progress_message(version, send, meta, std::move(data));
        TDCF_CHECK_SUCCESS(flag)
    }

    return StatusFlag::EventEnd;
}
