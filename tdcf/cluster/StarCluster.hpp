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

        ClusterFunOverride

    private:
        ProcessorAgentFactoryInherit(StarAgentFactory)

        void cluster_accept(unsigned cluster_size) override;

        void cluster_start() override;

        void cluster_end() override;

        static SerializablePtr create_node_data();

        StatusFlag handle_received_message(uint32_t from_id, const MetaData& meta, SerializablePtr& data) override;

        StatusFlag handle_disconnect_request(uint32_t from_id) override;

        using ProcessedData = Handle::ProgressTask;

        class Broadcast : public EventProgress {
        public:
            explicit Broadcast(ProgressType type, ProcessingRulesPtr rp);

            static StatusFlag create(ProcessingRulesPtr rp, Handle& info);

            StatusFlag handle_event(const MetaData& meta, Variant& data, Handle& info) override;

            void handle_error(Handle& info) override;

        protected:
            StatusFlag send_data(DataPtr& data, Handle& info);

            ProgressEventsMI _self;

            unsigned _sent = 0, _respond = 0;

        };

        class BroadcastAgent : public Broadcast, public EventProgressAgent {
        public:
            BroadcastAgent(ProcessingRulesPtr rp, ProgressEventsMI iter);

            static StatusFlag create(ProcessingRulesPtr rp, ProgressEventsMI other,
                                     Handle& info, EventProgressAgent **agent_ptr);

            StatusFlag handle_event(const MetaData& meta, Variant& data, Handle& info) override;

            void handle_error(Handle& info) override;

            StatusFlag proxy_event(const MetaData& meta, Variant& data, Handle& info) override;

        private:
            StatusFlag close(Handle& info) const;

            ProgressEventsMI _other;

        };

        class Scatter : public EventProgress {
        public:
            explicit Scatter(ProgressType type, ProcessingRulesPtr rp);

            static StatusFlag create(ProcessingRulesPtr rp, Handle& info);

            StatusFlag handle_event(const MetaData& meta, Variant& data, Handle& info) override;

            void handle_error(Handle& info) override;

        protected:
            StatusFlag scatter_data(DataPtr& data, Handle& info) const;

            StatusFlag send_data(unsigned offset, DataSet& set, Handle& info);

            ProgressEventsMI _self;

            unsigned _sent = 0, _respond = 0;

        };

        class ScatterAgent : public Scatter, public EventProgressAgent {
        public:
            ScatterAgent(ProcessingRulesPtr rp, ProgressEventsMI iter);

            static StatusFlag create(ProcessingRulesPtr rp, ProgressEventsMI other,
                                     Handle& info, EventProgressAgent **agent_ptr);

            StatusFlag handle_event(const MetaData& meta, Variant& data, Handle& info) override;

            void handle_error(Handle& info) override;

            StatusFlag proxy_event(const MetaData& meta, Variant& data, Handle& info) override;

        private:
            StatusFlag close(Handle& info) const;

            ProgressEventsMI _other;

        };

        class Reduce : public EventProgress {
        public:
            explicit Reduce(ProgressType type, ProcessingRulesPtr rp);

            static StatusFlag create(ProcessingRulesPtr rp, Handle& info);

            StatusFlag handle_event(const MetaData& meta, Variant& data, Handle& info) override;

            void handle_error(Handle& info) override;

        protected:
            StatusFlag acquire_data(DataPtr& data, Handle& info);

            ProgressEventsMI _self;

            DataSet _set;

        };

        class ReduceAgent : public Reduce, public EventProgressAgent {
        public:
            ReduceAgent(ProcessingRulesPtr rp, ProgressEventsMI iter);

            static StatusFlag create(ProcessingRulesPtr rp, ProgressEventsMI other,
                                     Handle& info, EventProgressAgent **agent_ptr);

            StatusFlag handle_event(const MetaData& meta, Variant& data, Handle& info) override;

            void handle_error(Handle& info) override;

            StatusFlag proxy_event(const MetaData& meta, Variant& data, Handle& info) override;

        private:
            StatusFlag close(DataPtr& data, Handle& info) const;

            ProgressEventsMI _other;

        };

        class AllReduce : public EventProgress {
        public:
            explicit AllReduce(ProgressType type, ProcessingRulesPtr rp);

            static StatusFlag create(ProcessingRulesPtr rp, Handle& info);

            StatusFlag handle_event(const MetaData& meta, Variant& data, Handle& info) override;

            void handle_error(Handle& info) override;

        protected:
            StatusFlag acquire_data(const MetaData& meta, DataPtr& data, Handle& info);

            StatusFlag send_data(DataPtr& data, Handle& info);

            ProgressEventsMI _self;

            DataSet _set;

            /// 防止空数据。
            std::vector<bool> _get;

            unsigned _received = 0, _respond = 0;

        };

        class AllReduceAgent : public AllReduce, public EventProgressAgent {
        public:
            AllReduceAgent(ProcessingRulesPtr rp, ProgressEventsMI iter);

            static StatusFlag create(ProcessingRulesPtr rp, ProgressEventsMI other,
                                     Handle& info, EventProgressAgent **agent_ptr);

            StatusFlag handle_event(const MetaData& meta, Variant& data, Handle& info) override;

            void handle_error(Handle& info) override;

            StatusFlag proxy_event(const MetaData& meta, Variant& data, Handle& info) override;

        private:
            StatusFlag reduce_data(DataPtr& data, Handle& info);

            StatusFlag close(Handle& info);

            ProgressEventsMI _other;

        };

        class ReduceScatter : public EventProgress {
        public:
            explicit ReduceScatter(ProgressType type, ProcessingRulesPtr rp);

            static StatusFlag create(ProcessingRulesPtr rp, Handle& info);

            StatusFlag handle_event(const MetaData& meta, Variant& data, Handle& info) override;

            void handle_error(Handle& info) override;

        protected:
            StatusFlag acquire_data(const MetaData& meta, DataPtr& data, Handle& info);

            StatusFlag scatter_data(DataPtr& data, Handle& info);

            StatusFlag send_data(DataSet& set, Handle& info) const;

            ProgressEventsMI _self;

            DataSet _set;

            /// 防止空数据。
            std::vector<bool> _get;

            unsigned _received = 0, _respond = 0;

        };

        class ReduceScatterAgent : public ReduceScatter, public EventProgressAgent {
        public:
            explicit ReduceScatterAgent(ProcessingRulesPtr rp, ProgressEventsMI iter);

            static StatusFlag create(ProcessingRulesPtr rp, ProgressEventsMI other,
                                     Handle& info, EventProgressAgent **agent_ptr);

            StatusFlag handle_event(const MetaData& meta, Variant& data, Handle& info) override;

            void handle_error(Handle& info) override;

            StatusFlag proxy_event(const MetaData& meta, Variant& data, Handle& info) override;

        private:
            StatusFlag reduce_data(DataPtr& data, Handle& info);

            StatusFlag close(Handle& info);

            ProgressEventsMI _other;
        };

    };

}
