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
    // if (!info.red()) ++self._finish_count;
    // if (!info.black()) ++self._finish_count;

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
        return agent_store(std::get<DataPtr>(data), meta, handle);
    }
    if (meta.stage == N_Scatter::finish_ack) {
        _finish_ack = true;
        return close(handle);
    }
    if (meta.stage == Public_Broadcast::node_finish_ack) {
        _finish = true;
        return close(handle);
    }
    if (meta.stage == N_Scatter::send_rule) {
        return StatusFlag::Success;
    }
    TDCF_RAISE_ERROR(meta.stage error type)
}

StatusFlag DBTAgent::Scatter::agent_store(DataPtr& data,
                                          const MetaData& meta, Handle& handle) {
    return StatusFlag::Success;
}

StatusFlag DBTAgent::Scatter::close(Handle& handle) const {
    return StatusFlag::Success;
}
