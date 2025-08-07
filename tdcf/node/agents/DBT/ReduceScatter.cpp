//
// Created by taganyer on 25-7-31.
//

#include <tdcf/base/Errors.hpp>
#include <tdcf/base/types/DBT.hpp>
#include <tdcf/node/agents/DBT/DBTAgent.hpp>

using namespace tdcf;

using namespace tdcf::dbt;

DBTAgent::ReduceScatter::ReduceScatter(uint32_t version, ProcessingRulesPtr rp) :
    EventProgress(OperationType::ReduceScatter, ProgressType::NodeRoot, version, std::move(rp)) {}

StatusFlag DBTAgent::ReduceScatter::create(uint32_t version, const MetaData& meta,
                                           ProcessingRulesPtr rp, Handle& handle) {
    assert(meta.operation_type == OperationType::ReduceScatter);
    assert(meta.stage == N_ReduceScatter::get_rule);

    auto& info = handle.agent_data<DBTAgentData>();

    auto iter = handle.create_progress(std::make_unique<ReduceScatter>(version, std::move(rp)));

    auto& self = static_cast<ReduceScatter&>(*iter->second);
    self._self = iter;
    if (!info.red()) {
        ++self._receive1;
        ++self._get_rule;
    }
    if (!info.black()) {
        ++self._receive1;
        ++self._get_rule;
    }
    if (info.leaf2()) {
        self._finish_ack = 2;
    } else {
        if (!info.red()) ++self._finish_ack;
        if (!info.black()) ++self._finish_ack;
    }

    MetaData new_meta = self.create_meta();
    new_meta.stage = N_ReduceScatter::send_rule;
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

StatusFlag DBTAgent::ReduceScatter::handle_event(const MetaData& meta,
                                                 Variant& data, Handle& handle) {
    assert(meta.operation_type == OperationType::ReduceScatter);
    if (meta.stage == Public_ReduceScatter::node_acquire) {
        return acquire_self_data(std::get<DataSet>(data), handle);
    }
    if (meta.stage == N_ReduceScatter::acquire_data1) {
        return acquire_data1(std::get<DataPtr>(data), meta.rest_data, handle);
    }
    if (meta.stage == N_ReduceScatter::reduce_data) {
        return send_data1(std::get<DataSet>(data), handle);
    }
    if (meta.stage == N_ReduceScatter::acquire_data2) {
        return send_data2(std::get<DataPtr>(data), meta, handle);
    }
    if (meta.stage == N_ReduceScatter::finish_notify) {
        return get_notify(meta.data1[0], handle);
    }
    if (meta.stage == N_ReduceScatter::finish_ack) {
        ++_finish_ack;
        return close(handle);
    }
    if (meta.stage == Public_ReduceScatter::node_finish_ack) {
        _data_stored = true;
        return close(handle);
    }
    if (meta.stage == N_ReduceScatter::get_rule) {
        ++_get_rule;
        return close(handle);
    }
    TDCF_RAISE_ERROR(meta.stage error type)
}

StatusFlag DBTAgent::ReduceScatter::acquire_self_data(DataSet& dataset, Handle& handle) const {
    auto& info = handle.agent_data<DBTAgentData>();

    StatusFlag flag;

    MetaData meta = create_meta();
    meta.stage = N_Reduce::send_rule;
    flag = handle.send_progress_message(version, info.t1(), meta, rule);
    TDCF_CHECK_SUCCESS(flag)
    flag = handle.send_progress_message(version, info.t2(), meta, rule);
    TDCF_CHECK_SUCCESS(flag)

    meta.stage = N_ReduceScatter::send_data1;
    if (dataset.size() == 1) dataset.emplace_back(DataPtr());
    if (info.leaf1() && info.leaf2()) {
        uint32_t t1_rest_data = (dataset.size() + 1) / 2,
                 t2_rest_data = dataset.size() / 2;
        for (uint32_t i = 0; i < dataset.size(); ++i) {
            auto& data = dataset[i];
            if (i & 1) {
                meta.rest_data = --t2_rest_data;
                flag = handle.send_progress_message(version, info.t2(), meta, data);
            } else {
                meta.rest_data = --t1_rest_data;
                flag = handle.send_progress_message(version, info.t1(), meta, data);
            }
            TDCF_CHECK_SUCCESS(flag)
        }
        return close(handle);
    }
    if (info.leaf1()) {
        uint32_t rest_data = dataset.size();
        for (auto& data : dataset) {
            meta.rest_data = --rest_data;
            flag = handle.send_progress_message(version, info.t1(), meta, data);
            TDCF_CHECK_SUCCESS(flag)
        }
    } else {
        uint32_t rest_data = dataset.size();
        for (auto& data : dataset) {
            meta.rest_data = --rest_data;
            flag = handle.send_progress_message(version, info.t2(), meta, data);
            TDCF_CHECK_SUCCESS(flag)
        }
    }

    return close(handle);
}

StatusFlag DBTAgent::ReduceScatter::acquire_data1(DataPtr& data,
                                                  uint32_t rest_size, Handle& handle) {
    _set.emplace_back(std::move(data));
    if (rest_size == 0 && ++_receive1 == 2) {
        MetaData meta = create_meta();
        meta.stage = N_ReduceScatter::reduce_data;
        handle.reduce_data(_self, meta, rule, std::move(_set));
    }
    return StatusFlag::Success;
}

StatusFlag DBTAgent::ReduceScatter::send_data1(DataSet& dataset, Handle& handle) const {
    auto& info = handle.agent_data<DBTAgentData>();
    MetaData meta = create_meta();
    meta.stage = N_ReduceScatter::send_data1;

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

void DBTAgent::ReduceScatter::acquire_data2(DataPtr& data,
                                            const MetaData& meta, Handle& handle) {
    if (!_agent) {
        if (data->derived_type() != 0) {
            handle.store_data(rule, data);
        }
        if (meta.rest_data == 0 && ++_receive2 == 2) {
            _data_stored = true;
        }
    } else {
        _set.emplace_back(std::move(data));
        if (meta.rest_data == 0 && ++_receive2 == 2) {
            MetaData new_meta = create_meta();
            new_meta.stage = Public_ReduceScatter::node_store;
            Variant variant(std::move(_set));
            StatusFlag flag = _agent->proxy_event(new_meta, variant, handle);
            TDCF_CHECK_SUCCESS(flag)
        }
    }
}

StatusFlag DBTAgent::ReduceScatter::send_data2(DataPtr& data,
                                               const MetaData& meta, Handle& handle) {
    auto& info = handle.agent_data<DBTAgentData>();

    if (meta.serial == info.self_serial) {
        acquire_data2(data, meta, handle);
    } else {
        assert(info.internal1() || info.internal2());
        MetaData new_meta = create_meta();
        new_meta.serial = meta.serial;
        new_meta.rest_data = meta.rest_data;
        new_meta.stage = N_ReduceScatter::send_data2;

        bool in_red = false;
        if (meta.data1[0] == 1) {
            in_red = info.in_t1_red(meta.serial);
            new_meta.data1[0] = 1;
        } else {
            in_red = info.in_t2_red(meta.serial);
            new_meta.data1[0] = 0;
        }

        StatusFlag flag = handle.send_progress_message(version,
                                                       in_red ? info.red() : info.black(),
                                                       new_meta, data);
        TDCF_CHECK_SUCCESS(flag)
    }

    return close(handle);
}

StatusFlag DBTAgent::ReduceScatter::get_notify(bool receive_from_t1, Handle& handle) {
    auto& info = handle.agent_data<DBTAgentData>();
    StatusFlag flag;
    if (receive_from_t1) {
        _t1_end = true;
        if (info.internal1()) {
            MetaData meta = create_meta();
            meta.stage = N_ReduceScatter::finish_notify;
            meta.data1[0] = 1;
            if (info.red()) {
                flag = handle.send_progress_message(version, info.red(), meta, nullptr);
                TDCF_CHECK_SUCCESS(flag)
            }
            if (info.black()) {
                flag = handle.send_progress_message(version, info.black(), meta, nullptr);
                TDCF_CHECK_SUCCESS(flag)
            }
        }
    } else {
        _t2_end = true;
        if (info.internal2()) {
            MetaData meta = create_meta();
            meta.stage = N_ReduceScatter::finish_notify;
            meta.data1[0] = 0;
            if (info.red()) {
                flag = handle.send_progress_message(version, info.red(), meta, nullptr);
                TDCF_CHECK_SUCCESS(flag)
            }
            if (info.black()) {
                flag = handle.send_progress_message(version, info.black(), meta, nullptr);
                TDCF_CHECK_SUCCESS(flag)
            }
        }
    }
    return close(handle);
}

StatusFlag DBTAgent::ReduceScatter::close(Handle& handle) const {
    if (!_data_stored || !_t1_end || !_t2_end || _finish_ack != 2 || _get_rule != 3)
        return StatusFlag::Success;

    auto& info = handle.agent_data<DBTAgentData>();
    MetaData meta = create_meta();
    meta.stage = N_ReduceScatter::finish;

    StatusFlag flag = handle.send_progress_message(version, info.t2(), meta, nullptr);
    TDCF_CHECK_SUCCESS(flag)

    return StatusFlag::EventEnd;
}
