//
// Created by taganyer on 25-7-31.
//

#include <tdcf/base/Errors.hpp>
#include <tdcf/base/types/DBT.hpp>
#include <tdcf/node/agents/DBT/DBTAgent.hpp>

using namespace tdcf;

using namespace tdcf::dbt;

DBTAgent::AllReduce::AllReduce(uint32_t version, ProcessingRulesPtr rp) :
    EventProgress(OperationType::AllReduce, ProgressType::NodeRoot, version, std::move(rp)) {}

StatusFlag DBTAgent::AllReduce::create(uint32_t version, const MetaData& meta,
                                       ProcessingRulesPtr rp, Handle& handle) {
    assert(meta.operation_type == OperationType::AllReduce);
    assert(meta.stage == N_AllReduce::get_rule);

    auto& info = handle.agent_data<DBTAgentData>();

    auto iter = handle.create_progress(std::make_unique<AllReduce>(version, std::move(rp)));

    auto& self = static_cast<AllReduce&>(*iter->second);
    self._self = iter;
    if (!info.red()) ++self._receive1;
    if (!info.black()) ++self._receive1;

    if (info.leaf2()) self._finish_ack = true;

    MetaData new_meta = self.create_meta();
    new_meta.stage = N_AllReduce::send_rule;
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

StatusFlag DBTAgent::AllReduce::handle_event(const MetaData& meta,
                                             Variant& data, Handle& handle) {
    assert(meta.operation_type == OperationType::AllReduce);
    if (meta.stage == Public_AllReduce::node_acquire) {
        return acquire_self_data(std::get<DataSet>(data), handle);
    }
    if (meta.stage == N_AllReduce::acquire_data1) {
        return acquire_data1(std::get<DataPtr>(data), meta.rest_data, handle);
    }
    if (meta.stage == N_AllReduce::reduce_data) {
        return send_data1(std::get<DataSet>(data), handle);
    }
    if (meta.stage == N_AllReduce::acquire_data2) {
        return acquire_data2(std::get<DataPtr>(data), meta, handle);
    }
    if (meta.stage == N_AllReduce::finish_ack) {
        _finish_ack = true;
        return close(handle);
    }
    if (meta.stage == Public_AllReduce::node_finish_ack) {
        _data_stored = true;
        return close(handle);
    }
    if (meta.stage == N_AllReduce::get_rule) {
        return StatusFlag::Success;
    }
    TDCF_RAISE_ERROR(meta.stage error type)
}

StatusFlag DBTAgent::AllReduce::acquire_self_data(DataSet& dataset, Handle& handle) const {
    auto& info = handle.agent_data<DBTAgentData>();

    MetaData meta = create_meta();
    meta.stage = N_AllReduce::send_data1;

    if (dataset.size() == 1) dataset.emplace_back(DataPtr());
    if (info.leaf1() && info.leaf2()) {
        uint32_t t1_rest_data = (dataset.size() + 1) / 2,
                 t2_rest_data = dataset.size() / 2;
        for (uint32_t i = 0; i < dataset.size(); ++i) {
            auto& data = dataset[i];
            StatusFlag flag;
            if (i & 1) {
                meta.rest_data = --t2_rest_data;
                flag = handle.send_progress_message(version, info.t2(), meta, data);
            } else {
                meta.rest_data = --t1_rest_data;
                flag = handle.send_progress_message(version, info.t1(), meta, data);
            }
            TDCF_CHECK_SUCCESS(flag)
        }
        return StatusFlag::Success;
    }
    if (info.leaf1()) {
        uint32_t rest_data = dataset.size();
        for (auto& data : dataset) {
            meta.rest_data = --rest_data;
            StatusFlag flag = handle.send_progress_message(version, info.t1(), meta, data);
            TDCF_CHECK_SUCCESS(flag)
        }
    } else {
        uint32_t rest_data = dataset.size();
        for (auto& data : dataset) {
            meta.rest_data = --rest_data;
            StatusFlag flag = handle.send_progress_message(version, info.t2(), meta, data);
            TDCF_CHECK_SUCCESS(flag)
        }
    }

    return StatusFlag::Success;
}

StatusFlag DBTAgent::AllReduce::acquire_data1(DataPtr& data,
                                              uint32_t rest_size, Handle& handle) {
    _set.emplace_back(std::move(data));
    if (rest_size == 0 && ++_receive1 == 2) {
        MetaData meta = create_meta();
        meta.stage = N_AllReduce::reduce_data;
        handle.reduce_data(_self, meta, rule, std::move(_set));
    }
    return StatusFlag::Success;
}

StatusFlag DBTAgent::AllReduce::send_data1(DataSet& dataset, Handle& handle) const {
    auto& info = handle.agent_data<DBTAgentData>();
    MetaData meta = create_meta();
    meta.stage = N_AllReduce::send_data1;

    if (dataset.size() == 1) dataset.emplace_back(DataPtr());
    uint32_t rest_data = dataset.size();
    if (info.internal1()) {
        for (auto& data : dataset) {
            meta.rest_data = --rest_data;
            StatusFlag flag = handle.send_progress_message(version, info.t1(), meta, data);
            TDCF_CHECK_SUCCESS(flag)
        }
    } else {
        assert(info.internal2());
        for (auto& data : dataset) {
            meta.rest_data = --rest_data;
            StatusFlag flag = handle.send_progress_message(version, info.t2(), meta, data);
            TDCF_CHECK_SUCCESS(flag)
        }
    }

    return StatusFlag::Success;
}

StatusFlag DBTAgent::AllReduce::acquire_data2(DataPtr& data,
                                              const MetaData& meta, Handle& handle) {
    if (!_agent) {
        if (data->derived_type() != 0) {
            handle.store_data(rule, data);
        }
        if (meta.rest_data == 0 && ++_receive2 == 2) {
            _data_stored = true;
        }
    } else {
        agent_store(data, meta.rest_data, handle);
    }
    return send_data2(data, meta.rest_data, meta.data1[0], meta.data1[1], handle);
}

void DBTAgent::AllReduce::agent_store(DataPtr& data,
                                      uint32_t rest_size, Handle& handle) {
    _set.emplace_back(data);
    if (rest_size != 0 || ++_receive2 != 2) return;

    MetaData meta = create_meta();
    meta.stage = Public_AllReduce::node_store;
    // meta.rest_data = rest_size;

    Variant variant(std::move(_set));
    StatusFlag flag = _agent->proxy_event(meta, variant, handle);
    TDCF_CHECK_SUCCESS(flag)
}

StatusFlag DBTAgent::AllReduce::send_data2(DataPtr& data, uint32_t rest_size,
                                           bool receive_message_from_t1,
                                           uint32_t from_serial, Handle& handle) const {
    auto& info = handle.agent_data<DBTAgentData>();

    if (receive_message_from_t1 && info.leaf1() || !receive_message_from_t1 && info.leaf2()) {
        return close(handle);
    }

    MetaData meta = create_meta();
    meta.stage = N_AllReduce::send_data2;
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

StatusFlag DBTAgent::AllReduce::close(Handle& handle) const {
    if (!_data_stored || !_finish_ack)
        return StatusFlag::Success;

    auto& info = handle.agent_data<DBTAgentData>();
    MetaData meta = create_meta();
    meta.stage = N_AllReduce::finish;

    StatusFlag flag = handle.send_progress_message(version, info.t2(), meta, nullptr);
    TDCF_CHECK_SUCCESS(flag)

    return StatusFlag::EventEnd;
}
