//
// Created by taganyer on 25-6-15.
//
#pragma once

#include <tdcf/detail/EventProgress.hpp>
#include <tdcf/node/agents/NodeAgent.hpp>

namespace tdcf {

    class StarAgent : public NodeAgent {
    public:
        StatusFlag init(const MetaData& meta, Handle& handle) override;

        bool serialize(void *buffer, unsigned buffer_size) const override;

        bool deserialize(const void *buffer, unsigned buffer_size) override;

        [[nodiscard]] SerializableType derived_type() const override;

        [[nodiscard]] unsigned serialize_size() const override;

    private:
        StatusFlag create_progress(const MetaData& meta, ProcessingRulesPtr& rule,
                                   Handle& handle) override;

        StatusFlag end_agent(const MetaData& meta, Handle& handle) override;

        class Broadcast : public EventProgress {
        public:
            explicit Broadcast(uint32_t version, ProcessingRulesPtr rp);

            static StatusFlag create(const MetaData& meta, ProcessingRulesPtr rp, Handle& handle);

            StatusFlag handle_event(const MetaData& meta, Variant& data, Handle& handle) override;

        private:
            StatusFlag agent_store(Variant& data, Handle& handle) const;

            StatusFlag close(Handle& handle) const;

            EventProgressAgent *_agent = nullptr;

        };

        class Scatter : public EventProgress {
        public:
            explicit Scatter(uint32_t version, ProcessingRulesPtr rp);

            static StatusFlag create(const MetaData& meta, ProcessingRulesPtr rp, Handle& handle);

            StatusFlag handle_event(const MetaData& meta, Variant& data, Handle& handle) override;

        private:
            StatusFlag scatter_data(DataPtr& data, Handle& handle) const;

            StatusFlag agent_store(Variant& data, Handle& handle) const;

            StatusFlag close(Handle& handle) const;

            ProgressEventsMI _self;

            EventProgressAgent *_agent = nullptr;

        };

        class Reduce : public EventProgress {
        public:
            explicit Reduce(uint32_t version, ProcessingRulesPtr rp);

            static StatusFlag create(const MetaData& meta, ProcessingRulesPtr rp, Handle& handle);

            StatusFlag handle_event(const MetaData& meta, Variant& data, Handle& handle) override;

        private:
            StatusFlag close(DataPtr& data, Handle& handle) const;

            EventProgressAgent *_agent = nullptr;

        };

        class AllReduce : public EventProgress {
        public:
            explicit AllReduce(uint32_t version, ProcessingRulesPtr rp);

            static StatusFlag create(const MetaData& meta, ProcessingRulesPtr rp, Handle& handle);

            StatusFlag handle_event(const MetaData& meta, Variant& data, Handle& handle) override;

        private:
            StatusFlag acquire_data1(DataPtr& data, Handle& handle) const;

            StatusFlag acquire_data2(DataPtr& data, Handle& handle) const;

            StatusFlag close(Handle& handle) const;

            EventProgressAgent *_agent = nullptr;

        };

        class ReduceScatter : public EventProgress {
        public:
            explicit ReduceScatter(uint32_t version, ProcessingRulesPtr rp);

            static StatusFlag create(const MetaData& meta, ProcessingRulesPtr rp, Handle& handle);

            StatusFlag handle_event(const MetaData& meta, Variant& data, Handle& handle) override;

        private:
            StatusFlag acquire_data1(DataPtr& data, Handle& handle) const;

            StatusFlag acquire_data2(DataPtr& data, Handle& handle) const;

            StatusFlag close(Handle& handle) const;

            EventProgressAgent *_agent = nullptr;

        };

    };

}
