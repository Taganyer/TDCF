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
            IdentityPtr t1_parent;

            IdentityPtr t2_parent;

            IdentityPtr red_child;

            IdentityPtr black_child;

            bool is_leaf_node_in_t1;

            DBTAgentData(IdentityPtr t1_parent, IdentityPtr t2_parent,
                         IdentityPtr red_child, IdentityPtr black_child,
                         bool is_leaf_node_in_t1) :
                t1_parent(std::move(t1_parent)), t2_parent(std::move(t2_parent)),
                red_child(std::move(red_child)), black_child(std::move(black_child)),
                is_leaf_node_in_t1(is_leaf_node_in_t1) {};

        };

        static void create_agent_data(const IdentityPtr& from_id, Handle& handle);

        StatusFlag handle_disconnect(const IdentityPtr& id, Handle& handle) override;

        StatusFlag create_progress(uint32_t version, const MetaData& meta,
                                   ProcessingRulesPtr& rule, Handle& handle) override;

        class Broadcast : public EventProgress {
        public:
            explicit Broadcast(uint32_t version, ProcessingRulesPtr rp);

            static StatusFlag create(uint32_t version, const MetaData& meta, ProcessingRulesPtr rp, Handle& handle);

            StatusFlag handle_event(const MetaData& meta, Variant& data, Handle& handle) override;

        private:
            StatusFlag send_data(DataPtr& data, uint32_t rest_size,
                                 uint32_t from_serial, Handle& handle) const;

            StatusFlag agent_store(DataPtr& data, uint32_t rest_size, Handle& handle);

            StatusFlag close(bool receive_message_from_t1, Handle& handle);

            EventProgressAgent *_agent = nullptr;

            uint8_t _message_count = 0, _finish_count = 0;

            bool _t1_finished = false, _t2_finished = false;

            DataSet _set;

        };

    };

} // tdcf
