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

        using Iter = NodeInformation::ProgressEventsMI;

        class Broadcast : public EventProgress {
        public:
            static StatusFlag create(ProcessingRulesPtr rp, NodeInformation& info);

            StatusFlag handle_event(const MetaData& meta, Variant& data, NodeInformation& node_info) override;

        private:
            DataPtr _data;

            Iter _iter;

            unsigned _sent = 0, _respond = 0;

            explicit Broadcast(ProcessingRulesPtr rp);

            StatusFlag send(NodeInformation& node_info);

        };

        class Scatter : public EventProgress {
        public:
            explicit Scatter(ProcessingRulesPtr rp);

            StatusFlag handle_event(const MetaData& meta, Variant& data, NodeInformation& node_info) override;

        };

        class Reduce : public EventProgress {
        public:
            explicit Reduce(ProcessingRulesPtr rp);

            StatusFlag handle_event(const MetaData& meta, Variant& data, NodeInformation& node_info) override;

        };

        class AllGather : public EventProgress {
        public:
            explicit AllGather(ProcessingRulesPtr rp);

            StatusFlag handle_event(const MetaData& meta, Variant& data, NodeInformation& node_info) override;

        };

        class AllReduce : public EventProgress {
        public:
            explicit AllReduce(ProcessingRulesPtr rp);

            StatusFlag handle_event(const MetaData& meta, Variant& data, NodeInformation& node_info) override;

        };

        class ReduceScatter : public EventProgress {
        public:
            explicit ReduceScatter(ProcessingRulesPtr rp);

            StatusFlag handle_event(const MetaData& meta, Variant& data, NodeInformation& node_info) override;

        };

        class AllToAll : public EventProgress {
        public:
            explicit AllToAll(ProcessingRulesPtr rp);

            StatusFlag handle_event(const MetaData& meta, Variant& data, NodeInformation& node_info) override;

        };

    };

}
