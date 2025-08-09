//
// Created by taganyer on 25-6-21.
//

#include <tdcf/base/Errors.hpp>
#include <tdcf/base/types/Star.hpp>
#include <tdcf/handle/Handle.hpp>
#include <tdcf/node/agents/star/StarAgent.hpp>

using namespace tdcf;

using namespace tdcf::star;

StarAgent::Reduce::Reduce(uint32_t version, ProcessingRulesPtr rp) :
    EventProgress(OperationType::Reduce, ProgressType::Node, version, std::move(rp)) {}

StatusFlag StarAgent::Reduce::create(uint32_t version, const MetaData& meta,
                                     ProcessingRulesPtr rp, Handle& handle) {
    assert(meta.operation_type == OperationType::Reduce);
    assert(meta.stage == N_Reduce::get_rule);

    auto iter = handle.create_progress(std::make_unique<Reduce>(version, std::move(rp)));

    auto& self = static_cast<Reduce&>(*iter->second);

    MetaData new_meta = self.create_meta();
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

StatusFlag StarAgent::Reduce::handle_event(const MetaData& meta,
                                           Variant& data, Handle& handle) {
    assert(meta.operation_type == OperationType::Reduce);
    if (meta.stage == Public_Reduce::node_acquire) {
        return close(std::get<DataSet>(data), handle);
    }
    TDCF_RAISE_ERROR(meta.stage error type)
}

StatusFlag StarAgent::Reduce::close(DataSet& dataset, Handle& handle) const {
    MetaData meta = create_meta();
    meta.stage = N_Reduce::send_data;
    meta.rest_data = dataset.size();

    auto& send = handle.agent_data<IdentityPtr>();
    for (auto& data : dataset) {
        --meta.rest_data;
        StatusFlag flag = handle.send_progress_message(version, send, meta, std::move(data));
        TDCF_CHECK_SUCCESS(flag)
    }

    return StatusFlag::EventEnd;
}
