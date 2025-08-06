//
// Created by taganyer on 25-5-25.
//

#include <variant>
#include <tdcf/base/Errors.hpp>
#include <tdcf/handle/Handle.hpp>
#include <tdcf/node/agents/NodeAgent.hpp>
#include <tdcf/node/agents/DBT/DBTAgent.hpp>
#include <tdcf/node/agents/ring/RingAgent.hpp>
#include <tdcf/node/agents/star/StarAgent.hpp>

using namespace tdcf;

StatusFlag NodeAgent::handle_received_message(const MetaData& meta,
                                              Variant& variant, Handle& handle) {
    if (meta.link_mark == LinkMark::Create) {
        assert(!handle.check_progress(handle.find_progress(meta.version)));

        auto& data_ptr = std::get<SerializablePtr>(variant);
        assert(data_ptr->base_type() == (int) SerializableBaseType::ProcessingRules);
        auto rule = std::dynamic_pointer_cast<ProcessingRules>(data_ptr);
        assert(rule);
        return create_progress(meta.version, meta, rule, handle);
    }

    auto iter = handle.find_progress(meta.version);
    TDCF_CHECK_EXPR(handle.check_progress(iter))
    return iter->second->handle_event(meta, variant, handle);
}

NodeAgentPtr tdcf::get_NodeAgent(const MetaData& meta) {
    assert(meta.operation_type == OperationType::AgentCreate);
    if (meta.data1[0] == ClusterType::star) {
        return std::make_shared<StarAgent>();
    }
    if (meta.data1[0] == ClusterType::ring) {
        return std::make_shared<RingAgent>();
    }
    if (meta.data1[0] == ClusterType::dbt) {
        return std::make_shared<DBTAgent>();
    }
    TDCF_RAISE_ERROR(error NodeAgent type)
}
