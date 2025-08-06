//
// Created by taganyer on 25-7-30.
//

#include <tdcf/base/Errors.hpp>
#include <tdcf/base/types/DBT.hpp>
#include <tdcf/node/agents/DBT/DBTAgent.hpp>

using namespace tdcf;

using namespace tdcf::dbt;

DBTAgent::Reduce::Reduce(uint32_t version, ProcessingRulesPtr rp) :
    EventProgress(OperationType::Reduce, ProgressType::NodeRoot, version, std::move(rp)) {}

StatusFlag DBTAgent::Reduce::create(uint32_t version, const MetaData& meta,
                                    ProcessingRulesPtr rp, Handle& handle) {
    assert(meta.operation_type == OperationType::Reduce);
    assert(meta.stage == N_Reduce::get_rule);

    auto& info = handle.agent_data<DBTAgentData>();

    auto iter = handle.create_progress(std::make_unique<Reduce>(version, std::move(rp)));

    auto& self = static_cast<Reduce&>(*iter->second);
    self._self = iter;
    if (!info.red()) {
        ++self._receive;
        ++self._get_rule;
    }
    if (!info.black()) {
        ++self._receive;
        ++self._get_rule;
    }

    MetaData new_meta = self.create_meta();
    new_meta.stage = N_Reduce::send_rule;
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

StatusFlag DBTAgent::Reduce::handle_event(const MetaData& meta,
                                          Variant& data, Handle& handle) {
    assert(meta.operation_type == OperationType::Reduce);

    if (meta.stage == Public_Reduce::node_acquire) {
        _get_self_data = true;
        return acquire_self_data(std::get<DataSet>(data), handle);
    }
    if (meta.stage == N_Reduce::acquire_data) {
        return acquire_data(std::get<DataPtr>(data), meta.rest_data, handle);
    }
    if (meta.stage == N_Reduce::reduce_data) {
        return send_data(std::get<DataSet>(data), handle);
    }
    if (meta.stage == N_Reduce::get_rule) {
        ++_get_rule;
        return close();
    }
    TDCF_RAISE_ERROR(meta.stage error type)
}

StatusFlag DBTAgent::Reduce::acquire_self_data(DataSet& dataset, Handle& handle) {
    auto& info = handle.agent_data<DBTAgentData>();

    StatusFlag flag;

    MetaData meta = create_meta();
    meta.stage = N_Reduce::send_rule;
    flag = handle.send_progress_message(version, info.t1(), meta, rule);
    TDCF_CHECK_SUCCESS(flag)
    flag = handle.send_progress_message(version, info.t2(), meta, rule);
    TDCF_CHECK_SUCCESS(flag)

    meta.stage = N_Reduce::send_data;
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
        _receive = 2;
        return close();
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

    return close();
}

StatusFlag DBTAgent::Reduce::acquire_data(DataPtr& data,
                                          uint32_t rest_size, Handle& handle) {
    _set.emplace_back(std::move(data));
    if (rest_size == 0 && ++_receive == 2) {
        MetaData meta = create_meta();
        meta.stage = N_Reduce::reduce_data;
        handle.reduce_data(_self, meta, rule, std::move(_set));
    }
    return StatusFlag::Success;
}

StatusFlag DBTAgent::Reduce::send_data(DataSet& dataset, Handle& handle) const {
    auto& info = handle.agent_data<DBTAgentData>();
    MetaData meta = create_meta();
    meta.stage = N_Reduce::send_data;

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

    return close();
}

StatusFlag DBTAgent::Reduce::close() const {
    if (!_get_self_data || _receive != 2 || _get_rule != 3)
        return StatusFlag::Success;
    return StatusFlag::EventEnd;
}
