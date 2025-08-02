//
// Created by taganyer on 25-6-15.
//
#pragma once

#include <tdcf/detail/EventProgress.hpp>
#include <tdcf/node/agents/NodeAgent.hpp>

namespace tdcf {

    class StarAgent : public NodeAgent {
    public:
        void init(const IdentityPtr& from_id, const MetaData& meta,
                  Handle& handle) override;

    private:
        using IdentityList = std::vector<IdentityPtr>;

        StatusFlag handle_disconnect(const IdentityPtr& id, Handle& handle) override;

        StatusFlag create_progress(uint32_t version, const MetaData& meta,
                                   ProcessingRulesPtr& rule, Handle& handle) override;

        class Broadcast : public EventProgress {
        public:
            explicit Broadcast(uint32_t version, ProcessingRulesPtr rp);

            static StatusFlag create(uint32_t version, const MetaData& meta, ProcessingRulesPtr rp, Handle& handle);

            StatusFlag handle_event(const MetaData& meta, Variant& data, Handle& handle) override;

        private:
            StatusFlag agent_store(DataPtr& data, uint32_t rest_size, Handle& handle);

            StatusFlag close(Handle& handle) const;

            EventProgressAgent *_agent = nullptr;

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

            ProgressEventsMI _self;

            EventProgressAgent *_agent = nullptr;

            DataSet _set;

        };

        class Reduce : public EventProgress {
        public:
            explicit Reduce(uint32_t version, ProcessingRulesPtr rp);

            static StatusFlag create(uint32_t version, const MetaData& meta, ProcessingRulesPtr rp, Handle& handle);

            StatusFlag handle_event(const MetaData& meta, Variant& data, Handle& handle) override;

        private:
            StatusFlag close(DataSet& dataset, Handle& handle) const;

            EventProgressAgent *_agent = nullptr;

        };

        class AllReduce : public EventProgress {
        public:
            explicit AllReduce(uint32_t version, ProcessingRulesPtr rp);

            static StatusFlag create(uint32_t version, const MetaData& meta, ProcessingRulesPtr rp, Handle& handle);

            StatusFlag handle_event(const MetaData& meta, Variant& data, Handle& handle) override;

        private:
            StatusFlag acquire_data1(DataSet& dataset, Handle& handle) const;

            StatusFlag acquire_data2(DataPtr& data, uint32_t rest_size, Handle& handle);

            StatusFlag close(Handle& handle) const;

            EventProgressAgent *_agent = nullptr;

            DataSet _set;

        };

        class ReduceScatter : public EventProgress {
        public:
            explicit ReduceScatter(uint32_t version, ProcessingRulesPtr rp);

            static StatusFlag create(uint32_t version, const MetaData& meta, ProcessingRulesPtr rp, Handle& handle);

            StatusFlag handle_event(const MetaData& meta, Variant& data, Handle& handle) override;

        private:
            StatusFlag acquire_data1(DataSet& dataset, Handle& handle) const;

            StatusFlag acquire_data2(DataPtr& data, uint32_t rest_size, Handle& handle);

            StatusFlag close(Handle& handle) const;

            EventProgressAgent *_agent = nullptr;

            DataSet _set;

        };

    };

}
