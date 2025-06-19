//
// Created by taganyer on 25-5-22.
//
#pragma once

#include <tdcf/cluster/Cluster.hpp>

namespace tdcf {

    class StarCluster : public Cluster {
    public:
        StarCluster(IdentityPtr ip, CommunicatorPtr cp, ProcessorPtr pp, IdentityPtr root_id) :
            Cluster(std::move(ip), std::move(cp), std::move(pp), std::move(root_id)) {};

        StatusFlag broadcast(ProcessingRulesPtr rule_ptr) override;

    private:
        void cluster_accept(unsigned cluster_size) override;

        void cluster_start() override;

        static SerializablePtr create_node_data();

        StatusFlag handle_received_message(IdentityPtr& id, const MetaData& meta, SerializablePtr& data) override;

        StatusFlag handle_disconnect_request(IdentityPtr& id) override;

        ProcessorAgentFactoryMacro(StarAgentFactory)


        using ProcessedData = NodeInformation::ProgressTask;

        class Broadcast : public EventProgress {
        public:
            explicit Broadcast(ProgressType type, ProcessingRulesPtr rp);

            static StatusFlag create(ProcessingRulesPtr rp, NodeInformation& info);

            StatusFlag handle_event(const MetaData& meta, Variant & data, NodeInformation& info) override;

        protected:
            StatusFlag send(NodeInformation& info);

            DataPtr _data;

            ProgressEventsMI _self;

            unsigned _sent = 0, _respond = 0;

        };

        class BroadcastAgent : public Broadcast, public EventProgressAgent {
        public:
            BroadcastAgent(ProcessingRulesPtr rp, ProgressEventsMI iter);

            static StatusFlag create(ProcessingRulesPtr rp, ProgressEventsMI other,
                                     NodeInformation& info, EventProgressAgent **agent_ptr);

            StatusFlag handle_event(const MetaData& meta, Variant & data, NodeInformation& info) override;

            StatusFlag store(const MetaData& meta, Variant& data, NodeInformation& info) override;

        private:
            StatusFlag close(NodeInformation& info) const;

            ProgressEventsMI _other;

        };

    };

}
