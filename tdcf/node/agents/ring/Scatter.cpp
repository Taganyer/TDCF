//
// Created by taganyer on 25-7-8.
//

#include <tdcf/base/Errors.hpp>
#include <tdcf/base/types/Ring.hpp>
#include <tdcf/handle/Handle.hpp>
#include <tdcf/node/agents/ring/RingAgent.hpp>

using namespace tdcf;

using namespace tdcf::ring;

RingAgent::Scatter::Scatter(uint32_t version, ProcessingRulesPtr rp) :
    EventProgress(OperationType::Scatter, ProgressType::NodeRoot, version, std::move(rp)) {}

StatusFlag RingAgent::Scatter::create(uint32_t version, const MetaData& meta,
                                      ProcessingRulesPtr rp, Handle& handle) {
    assert(meta.operation_type == OperationType::Scatter);
    assert(meta.stage == N_Scatter::get_rule);

    auto iter = handle.create_progress(std::make_unique<Scatter>(version, std::move(rp)));

    auto& [send, receive, serial] = handle.agent_data<RingAgentData>();
    assert(meta.serial == serial);

    auto& self = static_cast<Scatter&>(*iter->second);
    self.serial = serial - 1;
    self._self = iter;

    if (serial != 1) {
        MetaData new_meta = self.create_meta();
        new_meta.stage = N_Scatter::send_rule;
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

StatusFlag RingAgent::Scatter::handle_event(const MetaData& meta,
                                            Variant& data, Handle& handle) {
    assert(meta.operation_type == OperationType::Scatter);
    if (meta.stage == N_Scatter::get_data) {
        return agent_store(std::get<DataPtr>(data), meta.rest_data, handle);
    }
    if (meta.stage == N_Scatter::finish_ack) {
        _finish_ack = true;
        return close(handle);
    }
    if (meta.stage == Public_Broadcast::node_finish_ack) {
        _finish = true;
        return close(handle);
    }
    TDCF_RAISE_ERROR(meta.stage error type)
}

StatusFlag RingAgent::Scatter::agent_store(DataPtr& data,
                                           uint32_t rest_size, Handle& handle) {
    auto& [send, receive, serial] = handle.agent_data<RingAgentData>();
    if (rest_size != last) {
        last = rest_size;
        if (!_agent) {
            handle.store_data(rule, data);
            if (rest_size == 0) _finish = true;
        } else {
            _set.emplace_back(std::move(data));
            if (rest_size == 0) {
                MetaData meta = create_meta();
                meta.stage = Public_Scatter::node_store;
                Variant variant(std::move(_set));
                StatusFlag flag = _agent->proxy_event(meta, variant, handle);
                TDCF_CHECK_SUCCESS(flag)
            }
        }
    } else {
        MetaData meta = create_meta();
        meta.stage = N_Scatter::send_data;
        meta.rest_data = rest_size;
        StatusFlag flag = handle.send_progress_message(version, send, meta, data);
        TDCF_CHECK_SUCCESS(flag)
    }
    return StatusFlag::Success;
}

StatusFlag RingAgent::Scatter::close(Handle& handle) const {
    auto& [send, receive, serial] = handle.agent_data<RingAgentData>();
    if (!_finish_ack || !_finish) return StatusFlag::Success;
    assert(last == 0);
    MetaData meta = create_meta();
    meta.stage = N_Scatter::finish;
    StatusFlag flag = handle.send_progress_message(version, send, meta, nullptr);
    TDCF_CHECK_SUCCESS(flag)
    return StatusFlag::EventEnd;
}
