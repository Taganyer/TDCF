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

            bool is_leaf_node_in_t1, is_leaf_node_in_t2;

            uint32_t cluster_size = 0;

            uint32_t self_serial = static_cast<uint32_t>(-1);

            uint32_t red_serial = static_cast<uint32_t>(-1);

            uint32_t black_serial = static_cast<uint32_t>(-1);

            DBTAgentData(IdentityPtr t1_parent, IdentityPtr t2_parent,
                         IdentityPtr red_child, IdentityPtr black_child,
                         bool is_leaf_node_in_t1, bool is_leaf_node_in_t2);

            IdentityPtr& t1() { return t1_parent; };

            IdentityPtr& t2() { return t2_parent; };

            IdentityPtr& red() { return red_child; };

            IdentityPtr& black() { return black_child; };

            bool leaf1() const { return is_leaf_node_in_t1; };

            bool leaf2() const { return is_leaf_node_in_t2; };

            bool internal1() const { return !is_leaf_node_in_t1; };

            bool internal2() const { return !is_leaf_node_in_t2; };

            bool in_t1_red(uint32_t serial) const;

            bool in_t2_red(uint32_t serial) const;

        };

        bool _t1_connected = false, _t2_connected = false,
             _red_connected = false, _black_connected = false;

        void connect(bool connect, const IdentityPtr& id, Handle& handle);

        static void create_agent_data(const IdentityPtr& from_id, Handle& handle);

        static void disconnect(const IdentityPtr& id, Handle& handle);

        StatusFlag handle_disconnect(const IdentityPtr& id, Handle& handle) override;

        StatusFlag create_progress(uint32_t version, const MetaData& meta,
                                   ProcessingRulesPtr& rule, Handle& handle) override;

        class Broadcast : public EventProgress {
        public:
            explicit Broadcast(uint32_t version, ProcessingRulesPtr rp);

            static StatusFlag create(uint32_t version, const MetaData& meta, ProcessingRulesPtr rp, Handle& handle);

            StatusFlag handle_event(const MetaData& meta, Variant& data, Handle& handle) override;

        private:
            StatusFlag acquire_data(DataPtr& data, const MetaData& meta, Handle& handle);

            StatusFlag send_data(DataPtr& data, uint32_t rest_size, bool receive_message_from_t1,
                                 uint32_t from_serial, Handle& handle) const;

            void agent_store(DataPtr& data, uint32_t rest_size, Handle& handle);

            StatusFlag close(Handle& handle) const;

            EventProgressAgent *_agent = nullptr;

            uint8_t _receive = 0;

            bool _data_stored = false, _finish_ack = false;

            DataSet _set;

        };

        class Scatter : public EventProgress {
        public:
            explicit Scatter(uint32_t version, ProcessingRulesPtr rp);

            static StatusFlag create(uint32_t version, const MetaData& meta, ProcessingRulesPtr rp, Handle& handle);

            StatusFlag handle_event(const MetaData& meta, Variant& data, Handle& handle) override;

        private:
            void acquire_data(DataPtr& data, const MetaData& meta, Handle& handle);

            StatusFlag send_data(DataPtr& data, const MetaData& meta, Handle& handle);

            StatusFlag close(Handle& handle) const;

            EventProgressAgent *_agent = nullptr;

            ProgressEventsMI _self;

            DataSet _set;

            uint8_t _receive = 0;

            bool _data_stored = false, _finish_ack = false;

        };

        class Reduce : public EventProgress {
        public:
            explicit Reduce(uint32_t version, ProcessingRulesPtr rp);

            static StatusFlag create(uint32_t version, const MetaData& meta, ProcessingRulesPtr rp, Handle& handle);

            StatusFlag handle_event(const MetaData& meta, Variant& data, Handle& handle) override;

        private:
            StatusFlag acquire_self_data(DataSet& dataset, Handle& handle) const;

            StatusFlag acquire_data(DataPtr& data, uint32_t rest_size, Handle& handle);

            StatusFlag close(DataSet& dataset, Handle& handle) const;

            EventProgressAgent *_agent = nullptr;

            ProgressEventsMI _self;

            DataSet _set;

            uint8_t _receive = 0;

        };

        class AllReduce : public EventProgress {
        public:
            explicit AllReduce(uint32_t version, ProcessingRulesPtr rp);

            static StatusFlag create(uint32_t version, const MetaData& meta, ProcessingRulesPtr rp, Handle& handle);

            StatusFlag handle_event(const MetaData& meta, Variant& data, Handle& handle) override;

        private:
            StatusFlag acquire_self_data(DataSet& dataset, Handle& handle) const;

            StatusFlag acquire_data1(DataPtr& data, uint32_t rest_size, Handle& handle);

            StatusFlag send_data1(DataSet& dataset, Handle& handle) const;

            StatusFlag acquire_data2(DataPtr& data, const MetaData& meta, Handle& handle);

            void agent_store(DataPtr& data, uint32_t rest_size, Handle& handle);

            StatusFlag send_data2(DataPtr& data, uint32_t rest_size, bool receive_message_from_t1,
                                  uint32_t from_serial, Handle& handle) const;

            StatusFlag close(Handle& handle) const;

            EventProgressAgent *_agent = nullptr;

            ProgressEventsMI _self;

            DataSet _set;

            uint8_t _receive1 = 0,  _receive2 = 0;

            bool _data_stored = false, _finish_ack = false;

        };

        class ReduceScatter : public EventProgress {
        public:
            explicit ReduceScatter(uint32_t version, ProcessingRulesPtr rp);

            static StatusFlag create(uint32_t version, const MetaData& meta, ProcessingRulesPtr rp, Handle& handle);

            StatusFlag handle_event(const MetaData& meta, Variant& data, Handle& handle) override;

        private:
            StatusFlag acquire_self_data(DataSet& dataset, Handle& handle) const;

            StatusFlag acquire_data1(DataPtr& data, uint32_t rest_size, Handle& handle);

            StatusFlag send_data1(DataSet& dataset, Handle& handle) const;

            void acquire_data2(DataPtr& data, const MetaData& meta, Handle& handle);

            StatusFlag send_data2(DataPtr& data, const MetaData& meta, Handle& handle);

            StatusFlag close(Handle& handle) const;

            EventProgressAgent *_agent = nullptr;

            ProgressEventsMI _self;

            uint8_t _receive1 = 0,  _receive2 = 0;

            bool _data_stored = false, _finish_ack = false;

            DataSet _set;

        };

    };

} // tdcf
