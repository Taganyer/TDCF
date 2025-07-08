//
// Created by taganyer on 25-7-8.
//

#include <tdcf/base/Errors.hpp>
#include <tdcf/handle/Handle.hpp>
#include <tdcf/node/agents/ring/RingAgent.hpp>

using namespace tdcf;

RingAgent::Scatter::Scatter(uint32_t version, ProcessingRulesPtr rp) :
    EventProgress(OperationType::Scatter, ProgressType::NodeRoot, version, std::move(rp)) {}

StatusFlag RingAgent::Scatter::create(uint32_t version, const MetaData& meta,
                                      ProcessingRulesPtr rp, Handle& handle) {
    assert(meta.operation_type == OperationType::Scatter);
    assert(meta.stage == N_Scatter::get_rule);

    auto& [send, receive, serial] = handle.agent_data<RingAgentData>();
    assert(meta.serial == serial);

    auto iter = handle.create_progress(std::make_unique<Scatter>(version, std::move(rp)));

    auto& self = static_cast<Scatter&>(*iter->second);
    self.serial = serial - 1;
    self._self = iter;

    if (serial != 1) {
        MetaData new_meta = self.create_meta();
        new_meta.stage = C_Scatter::send_rule;
        StatusFlag flag = StatusFlag::Success;
        flag = handle.send_progress_message(version, send, new_meta, self.rule);
        if (flag != StatusFlag::Success) {
            handle.destroy_progress(iter);
            return flag;
        }
    }

    if (!handle.agent_factory) return StatusFlag::Success;

    StatusFlag flag = handle.agent_factory->scatter(self.rule, iter, handle, &self._agent);
    if (flag != StatusFlag::Success || !self._agent) {
        handle.destroy_progress(iter);
        return flag;
    }

    return StatusFlag::Success;
}

StatusFlag RingAgent::Scatter::handle_event(const MetaData& meta, Variant& data, Handle& handle) {
    assert(meta.operation_type == OperationType::Scatter);
    if (!_agent) {
        assert(meta.stage == N_Scatter::get_data);
        if (++get == 1) {
            handle.store_data(rule, std::get<DataPtr>(data));
            finish_ack = true;
        }
        return close(handle);
    }
    if (meta.stage == N_Scatter::get_data) {
        return agent_store(data, handle);
    }
    if (meta.stage == N_Scatter::finish_ack) {
        finish_ack = true;
        return close(handle);
    }
    TDCF_RAISE_ERROR(meta.stage error type)
}

StatusFlag RingAgent::Scatter::agent_store(Variant& data, Handle& handle) {
    auto& [send, receive, serial] = handle.agent_data<RingAgentData>();
    if (++get == 1) {
        MetaData meta = create_meta();
        meta.stage = N_Scatter::send_data;
        StatusFlag flag = _agent->proxy_event(meta, data, handle);
        TDCF_CHECK_SUCCESS(flag)
    } else {
        MetaData meta = create_meta();
        meta.stage = C_Scatter::send_data;
        StatusFlag flag = handle.send_progress_message(version, send, meta, std::get<DataPtr>(data));
        TDCF_CHECK_SUCCESS(flag)
    }
    return close(handle);
}

StatusFlag RingAgent::Scatter::close(Handle& handle) const {
    auto& [send, receive, serial] = handle.agent_data<RingAgentData>();
    if (get < serial || !finish_ack) return StatusFlag::Success;
    MetaData meta = create_meta();
    meta.stage = N_Scatter::finish;
    StatusFlag flag = handle.send_progress_message(version, send, meta, nullptr);
    TDCF_CHECK_SUCCESS(flag)
    return StatusFlag::EventEnd;
}
