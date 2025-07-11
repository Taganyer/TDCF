//
// Created by taganyer on 25-7-9.
//

#include <tdcf/base/Errors.hpp>
#include <tdcf/base/types/Ring.hpp>
#include <tdcf/handle/Handle.hpp>
#include <tdcf/node/agents/ring/RingAgent.hpp>

using namespace tdcf;

using namespace tdcf::ring;

RingAgent::ReduceScatter::ReduceScatter(uint32_t version, ProcessingRulesPtr rp) :
    EventProgress(OperationType::ReduceScatter, ProgressType::NodeRoot, version, std::move(rp)) {}

StatusFlag RingAgent::ReduceScatter::create(uint32_t version, const MetaData& meta,
                                            ProcessingRulesPtr rp, Handle& handle) {
    assert(meta.operation_type == OperationType::ReduceScatter);
    assert(meta.stage == N_ReduceScatter::get_rule);

    auto iter = handle.create_progress(std::make_unique<ReduceScatter>(version, std::move(rp)));

    auto& [send, receive, serial] = handle.agent_data<RingAgentData>();
    assert(serial == meta.serial);
    auto& self = static_cast<ReduceScatter&>(*iter->second);
    self.serial = meta.serial - 1;
    self._self = iter;

    MetaData new_meta = self.create_meta();
    if (serial != 1) {
        new_meta.stage = N_ReduceScatter::send_rule;
        StatusFlag flag = StatusFlag::Success;
        flag = handle.send_progress_message(version, send, new_meta, self.rule);
        if (flag != StatusFlag::Success) {
            handle.destroy_progress(iter);
            return flag;
        }
    }

    if (!handle.agent_factory) {
        new_meta.stage = Public_ReduceScatter::node_acquire;
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

StatusFlag RingAgent::ReduceScatter::handle_event(const MetaData& meta,
                                                  Variant& data, Handle& handle) {
    assert(meta.operation_type == OperationType::ReduceScatter);
    if (meta.stage == Public_ReduceScatter::node_acquire) {
        auto& set = std::get<DataSet>(data);
        uint32_t rest_size = set.size();
        for (auto& d : set) {
            --rest_size;
            acquire_data1(d, rest_size, handle);
        }
        return StatusFlag::Success;
    }
    if (meta.stage == N_ReduceScatter::get_data1) {
        return acquire_data1(std::get<DataPtr>(data), meta.rest_data, handle);
    }
    if (meta.stage == N_ReduceScatter::reduce_data) {
        return send_data1(std::get<DataSet>(data), handle);
    }
    if (meta.stage == N_ReduceScatter::get_data2) {
        return acquire_data2(std::get<DataPtr>(data), meta.rest_data, handle);
    }
    if (meta.stage == N_ReduceScatter::finish_ack) {
        _finish_ack = true;
        return close(handle);
    }
    if (meta.stage == Public_ReduceScatter::node_finish_ack) {
        _finish = true;
        return close(handle);
    }
    TDCF_RAISE_ERROR(meta.stage error type)
}

StatusFlag RingAgent::ReduceScatter::acquire_data1(DataPtr& data, uint32_t rest_size, Handle& handle) {
    _set.emplace_back(std::move(data));
    if (rest_size == 0) ++_get;

    if (_get == 2) {
        auto& [send, receive, serial] = handle.agent_data<RingAgentData>();
        MetaData meta = create_meta();
        meta.stage = N_ReduceScatter::reduce_data;
        handle.reduce_data(_self, meta, rule, _set);
    }

    return StatusFlag::Success;
}

StatusFlag RingAgent::ReduceScatter::send_data1(DataSet& dataset, Handle& handle) const {
    auto& [send, receive, serial] = handle.agent_data<RingAgentData>();
    MetaData meta = create_meta();
    meta.stage = N_ReduceScatter::send_data1;
    meta.rest_data = dataset.size();

    for (auto& data : dataset) {
        --meta.rest_data;
        StatusFlag flag = handle.send_progress_message(version, send, meta, std::move(data));
        TDCF_CHECK_SUCCESS(flag)
    }

    return StatusFlag::Success;
}

StatusFlag RingAgent::ReduceScatter::acquire_data2(DataPtr& data, uint32_t rest_size, Handle& handle) {
    auto& [send, receive, serial] = handle.agent_data<RingAgentData>();
    if (rest_size != _last) {
        _last = rest_size;
        if (!_agent) {
            handle.store_data(rule, data);
            if (rest_size == 0) _finish = true;
        } else {
            _set.emplace_back(std::move(data));
            if (rest_size == 0) {
                MetaData meta = create_meta();
                meta.stage = Public_ReduceScatter::node_store;
                Variant variant(std::move(_set));
                StatusFlag flag = _agent->proxy_event(meta, variant, handle);
                TDCF_CHECK_SUCCESS(flag)
            }
        }
    } else {
        MetaData meta = create_meta();
        meta.stage = N_ReduceScatter::send_data2;
        meta.rest_data = rest_size;
        StatusFlag flag = handle.send_progress_message(version, send, meta, data);
        TDCF_CHECK_SUCCESS(flag)
    }
    return StatusFlag::Success;
}

StatusFlag RingAgent::ReduceScatter::close(Handle& handle) const {
    auto& [send, receive, serial] = handle.agent_data<RingAgentData>();
    if (!_finish_ack || !_finish) return StatusFlag::Success;
    MetaData meta = create_meta();
    meta.stage = N_ReduceScatter::finish;
    StatusFlag flag = handle.send_progress_message(version, send, meta, nullptr);
    TDCF_CHECK_SUCCESS(flag)
    return StatusFlag::EventEnd;
}
