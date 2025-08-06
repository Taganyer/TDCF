//
// Created by taganyer on 25-7-6.
//
#pragma once

#include <tdcf/cluster/Cluster.hpp>

namespace tdcf {

    class RingCluster : public Cluster {
    public:
        RingCluster(IdentityPtr ip, CommunicatorPtr cp, ProcessorPtr pp) :
            Cluster(std::move(ip), std::move(cp), std::move(pp)) {};

        ClusterFunOverride

    private:
        struct RingClusterData {
            IdentityPtr send;
            IdentityPtr receive;
            uint32_t cluster_size;

            RingClusterData(IdentityPtr send, IdentityPtr receive, uint32_t cluster_size) :
                send(std::move(send)), receive(std::move(receive)), cluster_size(cluster_size) {};
        };

        ProcessorAgentFactoryInherit(RingAgentFactory)

        void cluster_connect_children(const IdentitySet& child_nodes) override;

        void waiting_respond();

        void cluster_start() override;

        void cluster_end() override;

        bool from_sub_cluster(const IdentityPtr& from_id) override;

        StatusFlag handle_disconnect_request(const IdentityPtr& from_id) override;

        class Broadcast : public EventProgress {
        public:
            explicit Broadcast(ProgressType type, uint32_t version, ProcessingRulesPtr rp);

            static StatusFlag create(ProcessingRulesPtr rp, Handle& handle);

            StatusFlag handle_event(const MetaData& meta, Variant& data, Handle& handle) override;

        protected:
            StatusFlag send_data(DataSet& dataset, Handle& handle) const;

        };

        class BroadcastAgent : public Broadcast, public EventProgressAgent {
        public:
            BroadcastAgent(uint32_t version, ProcessingRulesPtr rp, ProgressEventsMI iter);

            static StatusFlag create(ProcessingRulesPtr rp, ProgressEventsMI other,
                                     Handle& handle, EventProgressAgent **agent_ptr);

            StatusFlag handle_event(const MetaData& meta, Variant& data, Handle& handle) override;


            StatusFlag proxy_event(const MetaData& meta, Variant& data, Handle& handle) override;

        private:
            StatusFlag close(Handle& handle) const;

            ProgressEventsMI _other;

        };

        class Scatter : public EventProgress {
        public:
            explicit Scatter(ProgressType type, uint32_t version, ProcessingRulesPtr rp);

            static StatusFlag create(ProcessingRulesPtr rp, Handle& handle);

            StatusFlag handle_event(const MetaData& meta, Variant& data, Handle& handle) override;

        protected:
            StatusFlag scatter_data(DataSet& dataset, Handle& handle) const;

            StatusFlag send_data(DataSet& set, Handle& handle) const;

            ProgressEventsMI _self;

        };

        class ScatterAgent : public Scatter, public EventProgressAgent {
        public:
            ScatterAgent(uint32_t version, ProcessingRulesPtr rp, ProgressEventsMI iter);

            static StatusFlag create(ProcessingRulesPtr rp, ProgressEventsMI other,
                                     Handle& handle, EventProgressAgent **agent_ptr);

            StatusFlag handle_event(const MetaData& meta, Variant& data, Handle& handle) override;


            StatusFlag proxy_event(const MetaData& meta, Variant& data, Handle& handle) override;

        private:
            StatusFlag close(Handle& handle) const;

            ProgressEventsMI _other;

            DataSet _set;

        };

        class Reduce : public EventProgress {
        public:
            explicit Reduce(ProgressType type, uint32_t version, ProcessingRulesPtr rp);

            static StatusFlag create(ProcessingRulesPtr rp, Handle& handle);

            StatusFlag handle_event(const MetaData& meta, Variant& data, Handle& handle) override;

        protected:
            StatusFlag acquire_data(DataSet& dataset, Handle& handle) const;

        };

        class ReduceAgent : public Reduce, public EventProgressAgent {
        public:
            ReduceAgent(uint32_t version, ProcessingRulesPtr rp, ProgressEventsMI iter);

            static StatusFlag create(ProcessingRulesPtr rp, ProgressEventsMI other,
                                     Handle& handle, EventProgressAgent **agent_ptr);

            StatusFlag handle_event(const MetaData& meta, Variant& data, Handle& handle) override;


            StatusFlag proxy_event(const MetaData& meta, Variant& data, Handle& handle) override;

        private:
            StatusFlag close(DataPtr& data, uint32_t rest_size, Handle& handle);

            ProgressEventsMI _other;

            DataSet _set;

        };

        class AllReduce : public EventProgress {
        public:
            explicit AllReduce(ProgressType type, uint32_t version, ProcessingRulesPtr rp);

            static StatusFlag create(ProcessingRulesPtr rp, Handle& handle);

            StatusFlag handle_event(const MetaData& meta, Variant& data, Handle& handle) override;

        protected:
            StatusFlag acquire_data(DataSet& dataset, Handle& handle) const;

            StatusFlag send_data(DataPtr& data, uint32_t rest_size, Handle& handle) const;

        };

        class AllReduceAgent : public AllReduce, public EventProgressAgent {
        public:
            AllReduceAgent(uint32_t version, ProcessingRulesPtr rp, ProgressEventsMI iter);

            static StatusFlag create(ProcessingRulesPtr rp, ProgressEventsMI other,
                                     Handle& handle, EventProgressAgent **agent_ptr);

            StatusFlag handle_event(const MetaData& meta, Variant& data, Handle& handle) override;


            StatusFlag proxy_event(const MetaData& meta, Variant& data, Handle& handle) override;

        private:
            StatusFlag agent_get_data(DataPtr& data, uint32_t rest_size, Handle& handle);

            StatusFlag close(Handle& handle) const;

            ProgressEventsMI _other;

            DataSet _set;

        };

        class ReduceScatter : public EventProgress {
        public:
            explicit ReduceScatter(ProgressType type, uint32_t version, ProcessingRulesPtr rp);

            static StatusFlag create(ProcessingRulesPtr rp, Handle& handle);

            StatusFlag handle_event(const MetaData& meta, Variant& data, Handle& handle) override;

        protected:
            StatusFlag acquire_data(DataSet& dataset, Handle& handle) const;

            StatusFlag scatter_data(DataPtr& data, uint32_t rest_size, Handle& handle);

            StatusFlag send_data(DataSet& set, Handle& handle) const;

            ProgressEventsMI _self;

            DataSet _set;

        };

        class ReduceScatterAgent : public ReduceScatter, public EventProgressAgent {
        public:
            explicit ReduceScatterAgent(uint32_t version, ProcessingRulesPtr rp, ProgressEventsMI iter);

            static StatusFlag create(ProcessingRulesPtr rp, ProgressEventsMI other,
                                     Handle& handle, EventProgressAgent **agent_ptr);

            StatusFlag handle_event(const MetaData& meta, Variant& data, Handle& handle) override;


            StatusFlag proxy_event(const MetaData& meta, Variant& data, Handle& handle) override;

        private:
            StatusFlag agent_get_data(DataPtr& data, uint32_t rest_size, Handle& handle);

            StatusFlag close(Handle& handle) const;

            ProgressEventsMI _other;

        };

    };

}
