//
// Created by taganyer on 25-5-22.
//
#pragma once

#include <tdcf/cluster/Cluster.hpp>

namespace tdcf {

    class StarCluster : public Cluster {
    public:
        StarCluster(IdentityPtr ip, CommunicatorPtr cp, ProcessorPtr pp) :
            Cluster(std::move(ip), std::move(cp), std::move(pp)) {};

        ClusterFunOverride

    private:
        ProcessorAgentFactoryInherit(StarAgentFactory)

        using IdentityList = std::vector<IdentityPtr>;

        void cluster_connect_children(const IdentitySet& child_nodes) override;

        void cluster_start() override;

        void cluster_end() override;

        bool come_from_children(const IdentityPtr& from_id) override;

        static SerializablePtr create_node_data();

        StatusFlag handle_disconnect_request(const IdentityPtr& from_id) override;


        class Broadcast : public EventProgress {
        public:
            explicit Broadcast(ProgressType type, uint32_t version, ProcessingRulesPtr rp);

            static StatusFlag create(ProcessingRulesPtr rp, Handle& handle);

            StatusFlag handle_event(const MetaData& meta, Variant& data, Handle& handle) override;

        protected:
            StatusFlag send_data(DataPtr& data, Handle& handle) const;

            unsigned _sent = 0, _respond = 0;

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
            StatusFlag scatter_data(DataPtr& data, Handle& handle) const;

            StatusFlag send_data(DataSet& set, Handle& handle);

            ProgressEventsMI _self;

            unsigned _sent = 0, _respond = 0;

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

        };

        class Reduce : public EventProgress {
        public:
            explicit Reduce(ProgressType type, uint32_t version, ProcessingRulesPtr rp);

            static StatusFlag create(ProcessingRulesPtr rp, Handle& handle);

            StatusFlag handle_event(const MetaData& meta, Variant& data, Handle& handle) override;

        protected:
            StatusFlag acquire_data(DataPtr& data, Handle& handle);

            ProgressEventsMI _self;

            DataSet _set;

        };

        class ReduceAgent : public Reduce, public EventProgressAgent {
        public:
            ReduceAgent(uint32_t version, ProcessingRulesPtr rp, ProgressEventsMI iter);

            static StatusFlag create(ProcessingRulesPtr rp, ProgressEventsMI other,
                                     Handle& handle, EventProgressAgent **agent_ptr);

            StatusFlag handle_event(const MetaData& meta, Variant& data, Handle& handle) override;


            StatusFlag proxy_event(const MetaData& meta, Variant& data, Handle& handle) override;

        private:
            StatusFlag close(DataPtr& data, Handle& handle) const;

            ProgressEventsMI _other;

        };

        class AllReduce : public EventProgress {
        public:
            explicit AllReduce(ProgressType type, uint32_t version, ProcessingRulesPtr rp);

            static StatusFlag create(ProcessingRulesPtr rp, Handle& handle);

            StatusFlag handle_event(const MetaData& meta, Variant& data, Handle& handle) override;

        protected:
            StatusFlag acquire_data(const MetaData& meta, DataPtr& data, Handle& handle);

            StatusFlag send_data(DataPtr& data, Handle& handle);

            ProgressEventsMI _self;

            DataSet _set;

            /// 防止空数据。
            std::vector<bool> _get;

            unsigned _received = 0, _respond = 0;

        };

        class AllReduceAgent : public AllReduce, public EventProgressAgent {
        public:
            AllReduceAgent(uint32_t version, ProcessingRulesPtr rp, ProgressEventsMI iter);

            static StatusFlag create(ProcessingRulesPtr rp, ProgressEventsMI other,
                                     Handle& handle, EventProgressAgent **agent_ptr);

            StatusFlag handle_event(const MetaData& meta, Variant& data, Handle& handle) override;


            StatusFlag proxy_event(const MetaData& meta, Variant& data, Handle& handle) override;

        private:
            StatusFlag reduce_data(DataPtr& data, Handle& handle);

            StatusFlag close(Handle& handle) const;

            ProgressEventsMI _other;

        };

        class ReduceScatter : public EventProgress {
        public:
            explicit ReduceScatter(ProgressType type, uint32_t version, ProcessingRulesPtr rp);

            static StatusFlag create(ProcessingRulesPtr rp, Handle& handle);

            StatusFlag handle_event(const MetaData& meta, Variant& data, Handle& handle) override;

        protected:
            StatusFlag acquire_data(const MetaData& meta, DataPtr& data, Handle& handle);

            StatusFlag scatter_data(DataPtr& data, Handle& handle);

            StatusFlag send_data(DataSet& set, Handle& handle) const;

            ProgressEventsMI _self;

            DataSet _set;

            /// 防止空数据。
            std::vector<bool> _get;

            unsigned _received = 0, _respond = 0;

        };

        class ReduceScatterAgent : public ReduceScatter, public EventProgressAgent {
        public:
            explicit ReduceScatterAgent(uint32_t version, ProcessingRulesPtr rp, ProgressEventsMI iter);

            static StatusFlag create(ProcessingRulesPtr rp, ProgressEventsMI other,
                                     Handle& handle, EventProgressAgent **agent_ptr);

            StatusFlag handle_event(const MetaData& meta, Variant& data, Handle& handle) override;


            StatusFlag proxy_event(const MetaData& meta, Variant& data, Handle& handle) override;

        private:
            StatusFlag reduce_data(DataPtr& data, Handle& handle);

            StatusFlag close(Handle& handle) const;

            ProgressEventsMI _other;
        };

    };

}
