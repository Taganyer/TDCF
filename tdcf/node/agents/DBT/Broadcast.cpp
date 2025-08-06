//
// Created by taganyer on 25-7-23.
//

#include <tdcf/base/Errors.hpp>
#include <tdcf/base/types/DBT.hpp>
#include <tdcf/node/agents/DBT/DBTAgent.hpp>

using namespace tdcf;

using namespace tdcf::dbt;

DBTAgent::Broadcast::Broadcast(uint32_t version, ProcessingRulesPtr rp) :
    EventProgress(OperationType::Broadcast, ProgressType::NodeRoot, version, std::move(rp)) {}

StatusFlag DBTAgent::Broadcast::create(uint32_t version, const MetaData& meta,
                                       ProcessingRulesPtr rp, Handle& handle) {
    assert(meta.operation_type == OperationType::Broadcast);
    assert(meta.stage == N_Broadcast::get_rule);

    auto& info = handle.agent_data<DBTAgentData>();

    auto iter = handle.create_progress(std::make_unique<Broadcast>(version, std::move(rp)));

    auto& self = static_cast<Broadcast&>(*iter->second);
    if (info.leaf2()) {
        self._finish_ack = 2;
    } else {
        if (!info.red()) ++self._finish_ack;
        if (!info.black()) ++self._finish_ack;
    }

    MetaData new_meta = self.create_meta();
    new_meta.stage = N_Broadcast::send_rule;
    if (info.red()) {
        StatusFlag flag = handle.send_progress_message(version, info.red(), new_meta, self.rule);
        if (flag != StatusFlag::Success) {
            handle.destroy_progress(iter);
            return flag;
        }
    }
    if (info.black()) {
        StatusFlag flag = handle.send_progress_message(version, info.black(), new_meta, self.rule);
        if (flag != StatusFlag::Success) {
            handle.destroy_progress(iter);
            return flag;
        }
    }

    if (!handle.agent_factory) return StatusFlag::Success;

    StatusFlag flag = handle.agent_factory->broadcast(self.rule, iter, handle, &self._agent);
    if (flag != StatusFlag::Success || !self._agent) {
        handle.destroy_progress(iter);
        return flag;
    }

    return StatusFlag::Success;
}

StatusFlag DBTAgent::Broadcast::handle_event(const MetaData& meta,
                                             Variant& data, Handle& handle) {
    assert(meta.operation_type == OperationType::Broadcast);
    if (meta.stage == N_Broadcast::get_data) {
        return acquire_data(std::get<DataPtr>(data), meta, handle);
    }
    if (meta.stage == N_Broadcast::finish_ack) {
        ++_finish_ack;
        return close(handle);
    }
    if (meta.stage == Public_Broadcast::node_finish_ack) {
        _data_stored = true;
        return close(handle);
    }
    if (meta.stage == N_Broadcast::get_rule) {
        _get_rule = true;
        return close(handle);
    }
    TDCF_RAISE_ERROR(meta.stage error type)
}

StatusFlag DBTAgent::Broadcast::acquire_data(DataPtr& data,
                                             const MetaData& meta, Handle& handle) {
    if (!_agent) {
        if (data->derived_type() != 0) {
            handle.store_data(rule, data);
        }
        if (meta.rest_data == 0 && ++_receive == 2) {
            _data_stored = true;
        }
    } else {
        agent_store(data, meta.rest_data, handle);
    }
    return send_data(data, meta.rest_data, meta.data1[0], meta.data1[1], handle);
}

StatusFlag DBTAgent::Broadcast::send_data(DataPtr& data, uint32_t rest_size,
                                          bool receive_message_from_t1,
                                          uint32_t from_serial, Handle& handle) const {
    auto& info = handle.agent_data<DBTAgentData>();

    if (receive_message_from_t1 && info.leaf1() || !receive_message_from_t1 && info.leaf2()) {
        return close(handle);
    }

    MetaData meta = create_meta();
    meta.stage = N_Broadcast::send_data;
    meta.rest_data = rest_size;
    meta.data1[0] = info.internal1();

    StatusFlag flag;
    if (from_serial == 0) {
        if (info.black()) {
            meta.data1[1] = 1;
            flag = handle.send_progress_message(version, info.black(), meta, data);
            TDCF_CHECK_SUCCESS(flag)
        }
        if (info.red()) {
            meta.data1[1] = 0;
            flag = handle.send_progress_message(version, info.red(), meta, data);
        }
    } else {
        if (info.red()) {
            meta.data1[1] = 0;
            flag = handle.send_progress_message(version, info.red(), meta, data);
            TDCF_CHECK_SUCCESS(flag)
        }
        if (info.black()) {
            meta.data1[1] = 1;
            flag = handle.send_progress_message(version, info.black(), meta, data);
        }
    }
    TDCF_CHECK_SUCCESS(flag)

    return close(handle);
}

void DBTAgent::Broadcast::agent_store(DataPtr& data,
                                      uint32_t rest_size, Handle& handle) {
    _set.emplace_back(data);
    if (rest_size != 0 || ++_receive != 2) return;

    MetaData meta = create_meta();
    meta.stage = Public_Broadcast::node_store;
    meta.rest_data = rest_size;

    Variant variant(std::move(_set));
    StatusFlag flag = _agent->proxy_event(meta, variant, handle);
    TDCF_CHECK_SUCCESS(flag)
}

StatusFlag DBTAgent::Broadcast::close(Handle& handle) const {
    if (!_data_stored || _finish_ack != 2 || !_get_rule)
        return StatusFlag::Success;

    auto& info = handle.agent_data<DBTAgentData>();
    MetaData meta = create_meta();
    meta.stage = N_Broadcast::finish;

    StatusFlag flag = handle.send_progress_message(version, info.t2(), meta, nullptr);
    TDCF_CHECK_SUCCESS(flag)

    return StatusFlag::EventEnd;
}
