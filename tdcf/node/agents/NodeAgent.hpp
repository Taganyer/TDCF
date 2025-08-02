//
// Created by taganyer on 25-5-22.
//
#pragma once

#include <tdcf/detail/MetaData.hpp>
#include <tdcf/detail/EventProgress.hpp>
#include <tdcf/frame/Identity.hpp>
#include <tdcf/frame/ProcessingRules.hpp>

namespace tdcf {

    struct CommunicatorEvent;

    class Handle;

    class NodeAgent {
    public:
        virtual ~NodeAgent() = default;

        virtual void init(const IdentityPtr& from_id, const MetaData& meta,
                          Handle& handle) = 0;

        StatusFlag handle_received_message(const MetaData& meta,
                                           Variant& variant, Handle& handle);

        virtual StatusFlag handle_disconnect(const IdentityPtr& id, Handle& handle) = 0;

    protected:
        virtual StatusFlag create_progress(uint32_t version, const MetaData& meta,
                                           ProcessingRulesPtr& rule, Handle& handle) = 0;

    };

    using NodeAgentPtr = std::shared_ptr<NodeAgent>;

    NodeAgentPtr get_NodeAgent(const MetaData& meta);

}
