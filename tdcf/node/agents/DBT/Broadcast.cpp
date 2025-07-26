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

    auto& [t1, t2, red, black, leaf1] = handle.agent_data<DBTAgentData>();

    auto iter = handle.create_progress(std::make_unique<Broadcast>(version, std::move(rp)));

    auto& self = static_cast<Broadcast&>(*iter->second);
    if (!red || !black) ++self._finish_count;

    MetaData new_meta = self.create_meta();
    new_meta.stage = N_Broadcast::send_rule;
    if (!leaf1 && red) {
        StatusFlag flag = handle.send_progress_message(version, red, new_meta, self.rule);
        if (flag != StatusFlag::Success) {
            handle.destroy_progress(iter);
            return flag;
        }
    }
    if (!leaf1 && black) {
        StatusFlag flag = handle.send_progress_message(version, black, new_meta, self.rule);
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

StatusFlag DBTAgent::Broadcast::handle_event(const MetaData& meta, Variant& data, Handle& handle) {
    assert(meta.operation_type == OperationType::Broadcast);
    if (meta.stage == N_Broadcast::get_data) {
        if (!_agent) {
            if (std::get<DataPtr>(data)->derived_type() != 0) {
                handle.store_data(rule, std::get<DataPtr>(data));
            }
            if (meta.rest_data == 0) ++_message_count;
        } else {
            StatusFlag flag = agent_store(std::get<DataPtr>(data), meta.rest_data, handle);
            TDCF_CHECK_SUCCESS(flag)
        }

    }
}

StatusFlag DBTAgent::Broadcast::send_data(DataPtr& data, uint32_t rest_size,
                                          uint32_t from_serial, Handle& handle) const {
    auto& [t1, t2, red, black, leaf1] = handle.agent_data<DBTAgentData>();
    MetaData meta = create_meta();
    meta.stage = N_Broadcast::send_data;
    meta.rest_data = rest_size;
    meta.data1[0] = !leaf1;

    StatusFlag flag;
    /// TODO: 有BUG
    if (from_serial == 0) {
        meta.serial = 1;
        flag = handle.send_progress_message(version, black, meta, data);
        TDCF_CHECK_SUCCESS(flag)
        meta.serial = 0;
        flag = handle.send_progress_message(version, red, meta, data);
    } else {
        meta.serial = 0;
        flag = handle.send_progress_message(version, red, meta, data);
        TDCF_CHECK_SUCCESS(flag)
        meta.serial = 1;
        flag = handle.send_progress_message(version, black, meta, data);
    }
    TDCF_CHECK_SUCCESS(flag)

    return StatusFlag::Success;
}

StatusFlag DBTAgent::Broadcast::agent_store(DataPtr& data, uint32_t rest_size, Handle& handle) {
    _set.emplace_back(data);
    if (rest_size == 0) ++_message_count;
    if (_message_count != 2) return StatusFlag::Success;

    MetaData meta = create_meta();
    meta.stage = Public_Broadcast::node_store;
    meta.rest_data = rest_size;

    Variant variant(std::move(_set));
    return _agent->proxy_event(meta, variant, handle);
}

StatusFlag DBTAgent::Broadcast::close(bool receive_message_from_t1, Handle& handle) {
    auto& [t1, t2, red, black, leaf1] = handle.agent_data<DBTAgentData>();
    MetaData meta = create_meta();
    meta.stage = N_Broadcast::finish;
    StatusFlag flag = StatusFlag::Success;
    if (leaf1) {
        meta.data1[0] = 0;
        if (receive_message_from_t1) {
            _t1_finished = true;
            flag = handle.send_progress_message(version, t1, meta, nullptr);
        } else if (++_finish_count == 2) {
            _t2_finished = true;
            flag = handle.send_progress_message(version, t2, meta, nullptr);
        }
    } else {
        meta.data1[0] = 1;
        if (!receive_message_from_t1) {
            _t2_finished = true;
            flag = handle.send_progress_message(version, t2, meta, nullptr);
        } else if (++_finish_count == 2) {
            _t1_finished = true;
            flag = handle.send_progress_message(version, t1, meta, nullptr);
        }
    }
    TDCF_CHECK_SUCCESS(flag)
    if (_t1_finished && _t2_finished) return StatusFlag::ClusterOffline;
    return StatusFlag::Success;
}
