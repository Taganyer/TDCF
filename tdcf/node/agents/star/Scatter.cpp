//
// Created by taganyer on 25-6-20.
//

#include <tdcf/base/Errors.hpp>
#include <tdcf/base/types/Star.hpp>
#include <tdcf/handle/Handle.hpp>
#include <tdcf/node/agents/star/StarAgent.hpp>

using namespace tdcf;

using namespace tdcf::star;

StarAgent::Scatter::Scatter(uint32_t version, ProcessingRulesPtr rp) :
    EventProgress(OperationType::Scatter, ProgressType::Node, version, std::move(rp)) {}

StatusFlag StarAgent::Scatter::create(uint32_t version, const MetaData& meta,
                                      ProcessingRulesPtr rp, Handle& handle) {
    assert(meta.operation_type == OperationType::Scatter);
    assert(meta.stage == N_Scatter::get_rule);

    auto iter = handle.create_progress(std::make_unique<Scatter>(version, std::move(rp)));

    if (!handle.agent_factory) return StatusFlag::Success;

    auto& self = static_cast<Scatter&>(*iter->second);
    self._self = iter;

    StatusFlag flag = handle.agent_factory->scatter(self.rule, iter, handle, &self._agent);
    if (flag != StatusFlag::Success || !self._agent) {
        handle.destroy_progress(iter);
        return flag;
    }
    return StatusFlag::Success;
}

StatusFlag StarAgent::Scatter::handle_event(const MetaData& meta, Variant& data, Handle& handle) {
    assert(meta.operation_type == OperationType::Scatter);
    if (!_agent) {
        assert(meta.stage == N_Scatter::get_data);
        handle.store_data(rule, std::get<DataPtr>(data));
        if (meta.rest_data != 0) return StatusFlag::Success;
        return close(handle);
    }
    if (meta.stage == N_Scatter::get_data) {
        return agent_store(std::get<DataPtr>(data), meta.rest_data, handle);
    }
    /// 此时 _agent 指向的对象已销毁。
    if (meta.stage == Public_Scatter::node_finish_ack) {
        return close(handle);
    }
    TDCF_RAISE_ERROR(meta.stage error type)
}

StatusFlag StarAgent::Scatter::agent_store(DataPtr& data,
                                           uint32_t rest_size, Handle& handle) {
    _set.emplace_back(std::move(data));
    if (rest_size != 0) return StatusFlag::Success;
    MetaData meta = create_meta();
    meta.stage = Public_Scatter::node_store;
    Variant variant(std::move(_set));
    return _agent->proxy_event(meta, variant, handle);
}

StatusFlag StarAgent::Scatter::close(Handle& handle) const {
    MetaData meta = create_meta();
    meta.stage = N_Scatter::finish;
    assert(handle.has_agent_data() && handle.agent_data<IdentityPtr>());
    StatusFlag flag = handle.send_progress_message(version, handle.agent_data<IdentityPtr>(), meta, nullptr);
    TDCF_CHECK_SUCCESS(flag)
    return StatusFlag::EventEnd;
}
