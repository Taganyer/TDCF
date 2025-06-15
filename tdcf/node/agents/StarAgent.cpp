//
// Created by taganyer on 25-6-15.
//
#include <tdcf/base/Errors.hpp>
#include <tdcf/detail/NodeInformation.hpp>
#include <tdcf/node/agents/StarAgent.hpp>

using namespace tdcf;


StatusFlag StarAgent::Broadcast::create(ProcessingRulesPtr rp, NodeInformation& info) {
    MetaData meta(info.progress_events_version++, MetaDataTypes::Broadcast);
    auto [iter, success] = info.progress_events.emplace(
        meta, std::make_unique<Broadcast>(std::move(rp)));
    TDCF_CHECK_EXPR(success)

    auto& self = static_cast<Broadcast&>(*iter->second);
    StatusFlag flag = info.agent_factory->broadcast(self.rule, &self, info, &self._agent);
    if (flag != StatusFlag::Success) {
        info.progress_events.erase(iter);
        return flag;
    }
    return StatusFlag::Success;
}

StatusFlag StarAgent::Broadcast::handle_event(const MetaData& meta,
                                              Variant& data, NodeInformation& node_info) {
}

StarAgent::Broadcast::Broadcast(ProcessingRulesPtr rp) :
    EventProgress(EventType::CTNBroadcast, std::move(rp)) {}
