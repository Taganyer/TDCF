//
// Created by taganyer on 25-6-15.
//
#pragma once

#include <tdcf/detail/EventProgress.hpp>
#include <tdcf/node/agents/NodeAgent.hpp>

namespace tdcf {

    class StarAgent : public NodeAgent {
    public:
        StatusFlag init(const MetaData& meta, NodeInformation& info) override;

        StatusFlag serialize(void *buffer, unsigned buffer_size) const override;

        StatusFlag deserialize(const void *buffer, unsigned buffer_size) override;

        [[nodiscard]] SerializableType derived_type() const override;

        [[nodiscard]] unsigned serialize_size() const override;

    private:
        StatusFlag create_progress(const MetaData& meta, ProcessingRulesPtr& rule,
                                   NodeInformation& info) override;

        StatusFlag end_agent(const MetaData& meta, NodeInformation& info) override;

        class Broadcast : public EventProgress {
        public:
            explicit Broadcast(ProcessingRulesPtr rp, const MetaData& meta);

            static StatusFlag create(const MetaData& meta, ProcessingRulesPtr rp, NodeInformation& info);

            StatusFlag handle_event(const MetaData& meta, Variant& data, NodeInformation& info) override;

        private:
            StatusFlag agent_store(Variant& data, NodeInformation& info) const;

            StatusFlag close(NodeInformation& info) const;

            MetaData _root_meta;

            EventProgressAgent *_agent = nullptr;

        };

        class Scatter : public EventProgress {
        public:
            explicit Scatter(ProcessingRulesPtr rp, const MetaData& meta);

            static StatusFlag create(const MetaData& meta, ProcessingRulesPtr rp, NodeInformation& info);

            StatusFlag handle_event(const MetaData& meta, Variant& data, NodeInformation& info) override;

        private:
            StatusFlag scatter_data(DataPtr& data, NodeInformation& info) const;

            StatusFlag agent_store(Variant& data, NodeInformation& info) const;

            StatusFlag close(NodeInformation& info) const;

            MetaData _root_meta;

            ProgressEventsMI _self;

            EventProgressAgent *_agent = nullptr;

        };

        class Reduce : public EventProgress {
        public:
            explicit Reduce(ProcessingRulesPtr rp, const MetaData& meta);

            static StatusFlag create(const MetaData& meta, ProcessingRulesPtr rp, NodeInformation& info);

            StatusFlag handle_event(const MetaData& meta, Variant& data, NodeInformation& info) override;

        private:
            StatusFlag close(DataPtr& data, NodeInformation& info) const;

            MetaData _root_meta;

            ProgressEventsMI _self;

            EventProgressAgent *_agent = nullptr;

        };

        class AllReduce : public EventProgress {
        public:
            explicit AllReduce(ProcessingRulesPtr rp, const MetaData& meta);

            static StatusFlag create(const MetaData& meta, ProcessingRulesPtr rp, NodeInformation& info);

            StatusFlag handle_event(const MetaData& meta, Variant& data, NodeInformation& info) override;

        private:
            StatusFlag acquire_data1(DataPtr& data, NodeInformation& info) const;

            StatusFlag acquire_data2(DataPtr& data, NodeInformation& info) const;

            StatusFlag close(NodeInformation& info) const;

            MetaData _root_meta;

            ProgressEventsMI _self;

            EventProgressAgent *_agent = nullptr;

            unsigned _received = 0;

        };

    };

}
