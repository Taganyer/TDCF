//
// Created by taganyer on 25-6-20.
//

#include <tdcf/base/Errors.hpp>
#include <tdcf/handle/Handle.hpp>
#include <tdcf/node/agents/star/StarAgent.hpp>

using namespace tdcf;

StarAgent::Scatter::Scatter(uint32_t version, ProcessingRulesPtr rp) :
    EventProgress(OperationType::Scatter, ProgressType::NodeRoot, version, std::move(rp)) {}

StatusFlag StarAgent::Scatter::create(uint32_t version, const MetaData& meta,
                                      ProcessingRulesPtr rp, Handle& handle) {
    assert(meta.operation_type == OperationType::Scatter);
    assert(meta.stage == NodeAgentScatter::get_rule);

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
        assert(meta.stage == NodeAgentScatter::get_data);
        handle.store_data(rule, std::get<DataPtr>(data));
        return close(handle);
    }
    if (meta.stage == NodeAgentScatter::get_data) {
        return scatter_data(std::get<DataPtr>(data), handle);
    }
    if (meta.stage == NodeAgentScatter::scatter_data) {
        return agent_store(data, handle);
    }
    /// 此时 _agent 指向的对象已销毁。
    if (meta.stage == NodeAgentScatter::finish_ack) {
        return close(handle);
    }
    TDCF_RAISE_ERROR(meta.stage error type)
}

StatusFlag StarAgent::Scatter::scatter_data(DataPtr& data, Handle& handle) const {
    MetaData meta = create_meta();
    meta.stage = NodeAgentScatter::scatter_data;
    handle.scatter_data(_self, meta, rule, handle.cluster_size() + 1, data);
    return StatusFlag::Success;
}

StatusFlag StarAgent::Scatter::agent_store(Variant& data, Handle& handle) const {
    auto& set = std::get<DataSet>(data);
    assert(set.size() == handle.cluster_size() + 1);

    MetaData meta = create_meta();
    meta.stage = NodeAgentScatter::send_data;
    return _agent->proxy_event(meta, data, handle);
}

StatusFlag StarAgent::Scatter::close(Handle& handle) const {
    MetaData meta = create_meta();
    meta.stage = NodeAgentScatter::finish;
    assert(handle.root_identity());
    StatusFlag flag = handle.send_progress_message(version, handle.root_identity(), meta, nullptr);
    if (flag != StatusFlag::Success) return flag;
    return StatusFlag::EventEnd;
}
