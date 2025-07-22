//
// Created by taganyer on 25-7-16.
//
#pragma once

#include <tdcf/detail/EventProgress.hpp>
#include <tdcf/handle/Handle.hpp>
#include <tdcf/node/agents/NodeAgent.hpp>

namespace tdcf {

    class DBTAgent : public NodeAgent {
    public:
        void init(const IdentityPtr& from_id, const MetaData& meta,
                  Handle& handle) override;

        [[nodiscard]] SerializableType derived_type() const override;

    private:
        struct DBTAgentData {
            IdentityPtr red_child;

            IdentityPtr black_child;

            IdentityPtr t1_parent;

            IdentityPtr t2_parent;

            bool is_leaf_node_in_t1;

            DBTAgentData(IdentityPtr red_child, IdentityPtr black_child,
                         IdentityPtr t1_parent, IdentityPtr t2_parent,
                         bool is_leaf_node_in_t1) :
                red_child(std::move(red_child)), black_child(std::move(black_child)),
                t1_parent(std::move(t1_parent)), t2_parent(std::move(t2_parent)),
                is_leaf_node_in_t1(is_leaf_node_in_t1) {};

        };

        static void create_agent_data(const IdentityPtr& from_id, Handle& handle);

        StatusFlag handle_disconnect(const IdentityPtr& id, Handle& handle) override;

        StatusFlag create_progress(uint32_t version, const MetaData& meta,
                                   ProcessingRulesPtr& rule, Handle& handle) override;

    };

} // tdcf
