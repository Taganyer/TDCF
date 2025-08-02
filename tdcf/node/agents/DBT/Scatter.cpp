//
// Created by taganyer on 25-7-28.
//

#include <tdcf/base/Errors.hpp>
#include <tdcf/base/types/DBT.hpp>
#include <tdcf/node/agents/DBT/DBTAgent.hpp>

using namespace tdcf;

using namespace tdcf::dbt;

DBTAgent::Scatter::Scatter(uint32_t version, ProcessingRulesPtr rp) :
    EventProgress(OperationType::Scatter, ProgressType::NodeRoot, version, std::move(rp)) {}

StatusFlag DBTAgent::Scatter::create(uint32_t version, const MetaData& meta,
                                     ProcessingRulesPtr rp, Handle& handle) {
    assert(meta.operation_type == OperationType::Scatter);
    assert(meta.stage == N_Scatter::get_rule);

    auto& info = handle.agent_data<DBTAgentData>();

    auto iter = handle.create_progress(std::make_unique<Scatter>(version, std::move(rp)));

    auto& self = static_cast<Scatter&>(*iter->second);
    self._self = iter;
    if (info.leaf2()) {
        self._finish_ack = true;
    }

    MetaData new_meta = self.create_meta();
    new_meta.stage = N_Scatter::send_rule;
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

    StatusFlag flag = handle.agent_factory->scatter(self.rule, iter, handle, &self._agent);
    if (flag != StatusFlag::Success || !self._agent) {
        handle.destroy_progress(iter);
        return flag;
    }

    return StatusFlag::Success;
}

StatusFlag DBTAgent::Scatter::handle_event(const MetaData& meta,
                                           Variant& data, Handle& handle) {
    assert(meta.operation_type == OperationType::Scatter);
    if (meta.stage == N_Scatter::get_data) {
        return send_data(std::get<DataPtr>(data), meta, handle);
    }
    if (meta.stage == N_Scatter::finish_ack) {
        _finish_ack = true;
        return close(handle);
    }
    if (meta.stage == Public_Scatter::node_finish_ack) {
        _data_stored = true;
        return close(handle);
    }
    if (meta.stage == N_Scatter::send_rule) {
        return StatusFlag::Success;
    }
    TDCF_RAISE_ERROR(meta.stage error type)
}

void DBTAgent::Scatter::acquire_data(DataPtr& data, const MetaData& meta, Handle& handle) {
    if (!_agent) {
        if (data->derived_type() != 0) {
            handle.store_data(rule, data);
        }
        if (meta.rest_data == 0 && ++_receive == 2) {
            _data_stored = true;
        }
    } else {
        _set.emplace_back(std::move(data));
        if (meta.rest_data == 0 && ++_receive == 2) {
            MetaData new_meta = create_meta();
            new_meta.stage = Public_Scatter::node_store;
            Variant variant(std::move(_set));
            StatusFlag flag = _agent->proxy_event(new_meta, variant, handle);
            TDCF_CHECK_SUCCESS(flag)
        }
    }
}

StatusFlag DBTAgent::Scatter::send_data(DataPtr& data,
                                        const MetaData& meta, Handle& handle) {
    auto& info = handle.agent_data<DBTAgentData>();

    if (meta.serial == info.self_serial) {
        acquire_data(data, meta, handle);
    } else {
        assert(info.internal1() || info.internal2());
        MetaData new_meta = create_meta();
        new_meta.serial = meta.serial;
        new_meta.rest_data = meta.rest_data;
        new_meta.stage = N_Scatter::send_data;

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

StatusFlag DBTAgent::Scatter::close(Handle& handle) const {
    if (!_data_stored || !_finish_ack)
        return StatusFlag::Success;

    auto& info = handle.agent_data<DBTAgentData>();
    MetaData meta = create_meta();
    meta.stage = N_Scatter::finish;

    StatusFlag flag = handle.send_progress_message(version, info.t2(), meta, nullptr);
    TDCF_CHECK_SUCCESS(flag)

    return StatusFlag::EventEnd;
}
