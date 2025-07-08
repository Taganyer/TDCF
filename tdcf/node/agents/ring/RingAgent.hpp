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

        struct RingAgentData {
            IdentityPtr send;
            IdentityPtr receive;
            uint32_t serial;

            RingAgentData(IdentityPtr send, IdentityPtr receive, uint32_t serial) :
                send(std::move(send)), receive(std::move(receive)), serial(serial) {};
        };

    private:
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
            void send_data(DataPtr& data, Handle& handle) const;

            StatusFlag agent_store(Variant& data, Handle& handle) const;

            StatusFlag close(Handle& handle) const;

            EventProgressAgent *_agent = nullptr;

        };

        class Scatter : public EventProgress {
        public:
            explicit Scatter(uint32_t version, ProcessingRulesPtr rp);

            static StatusFlag create(uint32_t version, const MetaData& meta, ProcessingRulesPtr rp, Handle& handle);

            StatusFlag handle_event(const MetaData& meta, Variant& data, Handle& handle) override;

        private:
            StatusFlag agent_store(Variant& data, Handle& handle);

            StatusFlag close(Handle& handle) const;

            uint32_t get = 0;

            bool finish_ack = false;

            ProgressEventsMI _self;

            EventProgressAgent *_agent = nullptr;

        };

        class Reduce : public EventProgress {
        public:
            explicit Reduce(uint32_t version, ProcessingRulesPtr rp);

            static StatusFlag create(uint32_t version, const MetaData& meta, ProcessingRulesPtr rp, Handle& handle);

            StatusFlag handle_event(const MetaData& meta, Variant& data, Handle& handle) override;

        private:
            StatusFlag acquire_data(DataPtr& data, Handle& handle);

            StatusFlag close(DataPtr& data, Handle& handle) const;

            ProgressEventsMI _self;

            EventProgressAgent *_agent = nullptr;

            DataSet _set;

        };

    };

} // tdcf
