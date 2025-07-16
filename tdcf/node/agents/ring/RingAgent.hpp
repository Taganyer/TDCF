//
// Created by taganyer on 25-7-6.
//
#pragma once

#include <tdcf/detail/EventProgress.hpp>
#include <tdcf/handle/Handle.hpp>
#include <tdcf/node/agents/NodeAgent.hpp>

namespace tdcf {

    class RingAgent : public NodeAgent {
    public:
        void init(const IdentityPtr& from_id, const MetaData& meta,
                  Handle& handle) override;

        [[nodiscard]] SerializableType derived_type() const override;

    private:
        struct RingAgentData {
            IdentityPtr send;
            IdentityPtr receive;
            uint32_t serial;

            RingAgentData(IdentityPtr send, IdentityPtr receive, uint32_t serial) :
                send(std::move(send)), receive(std::move(receive)), serial(serial) {};
        };

        StatusFlag handle_disconnect(const IdentityPtr& id, Handle& handle) override;

        static void connect_handle(Handle::MessageEvent& event, Handle& handle);

        StatusFlag create_progress(uint32_t version, const MetaData& meta,
                                   ProcessingRulesPtr& rule, Handle& handle) override;

        class Broadcast : public EventProgress {
        public:
            explicit Broadcast(uint32_t version, ProcessingRulesPtr rp);

            static StatusFlag create(uint32_t version, const MetaData& meta, ProcessingRulesPtr rp, Handle& handle);

            StatusFlag handle_event(const MetaData& meta, Variant& data, Handle& handle) override;

        private:
            StatusFlag send_data(DataPtr& data, uint32_t rest_size, Handle& handle) const;

            StatusFlag agent_store(DataPtr& data, uint32_t rest_size, Handle& handle);

            StatusFlag close(Handle& handle) const;

            EventProgressAgent *_agent = nullptr;

            bool _finish_ack = false, _finish = false;

            DataSet _set;

        };

        class Scatter : public EventProgress {
        public:
            explicit Scatter(uint32_t version, ProcessingRulesPtr rp);

            static StatusFlag create(uint32_t version, const MetaData& meta, ProcessingRulesPtr rp, Handle& handle);

            StatusFlag handle_event(const MetaData& meta, Variant& data, Handle& handle) override;

        private:
            StatusFlag agent_store(DataPtr& data, uint32_t rest_size, Handle& handle);

            StatusFlag close(Handle& handle) const;

            EventProgressAgent *_agent = nullptr;

            ProgressEventsMI _self;

            DataSet _set;

            uint32_t last = -1;

            bool _finish_ack = false, _finish = false;

        };

        class Reduce : public EventProgress {
        public:
            explicit Reduce(uint32_t version, ProcessingRulesPtr rp);

            static StatusFlag create(uint32_t version, const MetaData& meta, ProcessingRulesPtr rp, Handle& handle);

            StatusFlag handle_event(const MetaData& meta, Variant& data, Handle& handle) override;

        private:
            StatusFlag acquire_data(DataPtr& data, uint32_t rest_size, Handle& handle);

            StatusFlag close(DataSet& dataset, Handle& handle) const;

            EventProgressAgent *_agent = nullptr;

            ProgressEventsMI _self;

            DataSet _set;

            uint32_t _step = 0;

        };

        class AllReduce : public EventProgress {
        public:
            explicit AllReduce(uint32_t version, ProcessingRulesPtr rp);

            static StatusFlag create(uint32_t version, const MetaData& meta, ProcessingRulesPtr rp, Handle& handle);

            StatusFlag handle_event(const MetaData& meta, Variant& data, Handle& handle) override;

        private:
            StatusFlag acquire_data1(DataPtr& data, uint32_t rest_size, Handle& handle);

            StatusFlag reduce_data(DataSet& dataset, Handle& handle) const;

            StatusFlag acquire_data2(DataPtr& data, uint32_t rest_size, Handle& handle);

            StatusFlag close(Handle& handle) const;

            EventProgressAgent *_agent = nullptr;

            ProgressEventsMI _self;

            DataSet _set;

            uint32_t _step = 0;

            bool _finish_ack = false, _finish = false;

        };

        class ReduceScatter : public EventProgress {
        public:
            explicit ReduceScatter(uint32_t version, ProcessingRulesPtr rp);

            static StatusFlag create(uint32_t version, const MetaData& meta, ProcessingRulesPtr rp, Handle& handle);

            StatusFlag handle_event(const MetaData& meta, Variant& data, Handle& handle) override;

        private:
            StatusFlag acquire_data1(DataPtr& data, uint32_t rest_size, Handle& handle);

            StatusFlag send_data1(DataSet& dataset, Handle& handle) const;

            StatusFlag acquire_data2(DataPtr& data, uint32_t rest_size, Handle& handle);

            StatusFlag close(Handle& handle) const;

            EventProgressAgent *_agent = nullptr;

            ProgressEventsMI _self;

            DataSet _set;

            uint32_t _get = 0, _last = -1;

            bool _finish_ack = false, _finish = false;

        };

    };

} // tdcf
