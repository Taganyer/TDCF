//
// Created by taganyer on 25-5-22.
//
#pragma once

#include <tdcf/cluster/Cluster.hpp>

namespace tdcf {

    class StarCluster : public Cluster {
    public:
        StarCluster(IdentityPtr ip, CommunicatorPtr cp, ProcessorPtr pp,
                    IdentityPtr root_id, unsigned cluster_size);

        ~StarCluster() override;

    private:
        SerializablePtr create_node_data();

        using ProcessedData = NodeInformation::ProgressTask;

        class Broadcast : public EventProgress {
        public:
            static StatusFlag create(ProcessingRulesPtr rp, NodeInformation& info);

            StatusFlag handle_event(const MetaData& meta, Variant *data, NodeInformation& node_info) override;

        protected:
            explicit Broadcast(EventType type, ProcessingRulesPtr rp);

            StatusFlag send(NodeInformation& node_info);

            DataPtr _data;

            ProgressEventsMI _self;

            unsigned _sent = 0, _respond = 0;

        };

        class BroadcastAgent : public Broadcast, public EventProgressAgent {
        public:
            static StatusFlag create(ProcessingRulesPtr rp, ProgressEventsMI other,
                                     NodeInformation& info, EventProgressAgent **agent_ptr);

            StatusFlag handle_event(const MetaData& meta, Variant *data, NodeInformation& node_info) override;

            StatusFlag store(const MetaData& meta, Variant *data, NodeInformation& node_info) override;

        private:
            StatusFlag close(NodeInformation& node_info) const;

            BroadcastAgent(ProcessingRulesPtr rp, ProgressEventsMI iter);

            ProgressEventsMI _other;

        };

        class Scatter : public EventProgress {
        public:
            explicit Scatter(ProcessingRulesPtr rp);

            StatusFlag handle_event(const MetaData& meta, Variant *data, NodeInformation& node_info) override;

        };

        class Reduce : public EventProgress {
        public:
            explicit Reduce(ProcessingRulesPtr rp);

            StatusFlag handle_event(const MetaData& meta, Variant *data, NodeInformation& node_info) override;

        };

        class AllGather : public EventProgress {
        public:
            explicit AllGather(ProcessingRulesPtr rp);

            StatusFlag handle_event(const MetaData& meta, Variant *data, NodeInformation& node_info) override;

        };

        class AllReduce : public EventProgress {
        public:
            explicit AllReduce(ProcessingRulesPtr rp);

            StatusFlag handle_event(const MetaData& meta, Variant *data, NodeInformation& node_info) override;

        };

        class ReduceScatter : public EventProgress {
        public:
            explicit ReduceScatter(ProcessingRulesPtr rp);

            StatusFlag handle_event(const MetaData& meta, Variant *data, NodeInformation& node_info) override;

        };

        class AllToAll : public EventProgress {
        public:
            explicit AllToAll(ProcessingRulesPtr rp);

            StatusFlag handle_event(const MetaData& meta, Variant *data, NodeInformation& node_info) override;

        };

    };

}
