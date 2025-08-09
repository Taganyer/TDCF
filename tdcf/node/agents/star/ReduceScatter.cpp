//
// Created by taganyer on 25-6-26.
//

#include <tdcf/base/Errors.hpp>
#include <tdcf/base/types/Star.hpp>
#include <tdcf/handle/Handle.hpp>
#include <tdcf/node/agents/star/StarAgent.hpp>

using namespace tdcf;

using namespace tdcf::star;

StarAgent::ReduceScatter::ReduceScatter(uint32_t version, ProcessingRulesPtr rp) :
    EventProgress(OperationType::ReduceScatter, ProgressType::Node, version, std::move(rp)) {}

StatusFlag StarAgent::ReduceScatter::create(uint32_t version, const MetaData& meta,
                                            ProcessingRulesPtr rp, Handle& handle) {
    assert(meta.operation_type == OperationType::ReduceScatter);
    assert(meta.stage == N_ReduceScatter::get_rule);

    auto iter = handle.create_progress(std::make_unique<ReduceScatter>(version, std::move(rp)));

    auto& self = static_cast<ReduceScatter&>(*iter->second);
    self.serial = meta.serial;

    MetaData new_meta = self.create_meta();
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

StatusFlag StarAgent::ReduceScatter::handle_event(const MetaData& meta,
                                                  Variant& data, Handle& handle) {
    assert(meta.operation_type == OperationType::ReduceScatter);
    if (meta.stage == Public_ReduceScatter::node_acquire) {
        return acquire_data1(std::get<DataSet>(data), handle);
    }
    if (meta.stage == N_ReduceScatter::acquire_data2) {
        return acquire_data2(std::get<DataPtr>(data), meta.rest_data, handle);
    }
    if (meta.stage == Public_ReduceScatter::node_finish_ack) {
        return close(handle);
    }
    TDCF_RAISE_ERROR(meta.stage error type)
}

StatusFlag StarAgent::ReduceScatter::acquire_data1(DataSet& dataset, Handle& handle) const {
    MetaData meta = create_meta();
    meta.stage = N_ReduceScatter::send_data1;
    meta.rest_data = dataset.size();

    for (auto& data : dataset) {
        --meta.rest_data;
        StatusFlag flag = handle.send_progress_message(version, handle.agent_data<IdentityPtr>(),
                                                       meta, std::move(data));
        TDCF_CHECK_SUCCESS(flag)
    }

    return StatusFlag::Success;
}

StatusFlag StarAgent::ReduceScatter::acquire_data2(DataPtr& data, uint32_t rest_size, Handle& handle) {
    if (!_agent) {
        handle.store_data(rule, data);
        if (rest_size != 0) return StatusFlag::Success;
        return close(handle);
    }
    _set.emplace_back(std::move(data));
    if (rest_size != 0) return StatusFlag::Success;

    MetaData meta = create_meta();
    meta.stage = Public_ReduceScatter::node_store;
    Variant variant = std::move(_set);
    StatusFlag flag = _agent->proxy_event(meta, variant, handle);
    TDCF_CHECK_SUCCESS(flag)
    return StatusFlag::Success;
}

StatusFlag StarAgent::ReduceScatter::close(Handle& handle) const {
    MetaData meta = create_meta();
    meta.stage = N_ReduceScatter::finish;
    StatusFlag flag = handle.send_progress_message(version, handle.agent_data<IdentityPtr>(), meta, nullptr);
    TDCF_CHECK_SUCCESS(flag)
    return StatusFlag::EventEnd;
}
