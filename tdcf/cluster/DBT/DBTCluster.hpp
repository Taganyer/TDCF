//
// Created by taganyer on 25-7-16.
//
#pragma once

#include <tdcf/cluster/Cluster.hpp>

namespace tdcf {

    namespace dbt {
        struct DBTInfo;
    }

    class DBTCluster : public Cluster {
    public:
        DBTCluster(IdentityPtr ip, CommunicatorPtr cp, ProcessorPtr pp) :
            Cluster(std::move(ip), std::move(cp), std::move(pp)) {};

        ClusterFunOverride

    private:
        struct DBTClusterData {
            IdentityPtr t1_root;

            IdentityPtr t2_root;

            uint32_t cluster_size;

            DBTClusterData(IdentityPtr t1_root, IdentityPtr t2_root, uint32_t cluster_size) :
                t1_root(std::move(t1_root)), t2_root(std::move(t2_root)), cluster_size(cluster_size) {};

        };

        ProcessorAgentFactoryInherit(DBTAgentFactory)

        void cluster_connect_children(const IdentitySet& child_nodes) override;

        void cluster_start() override;

        void send_message_to_child(const std::vector<IdentityPtr>& node_list,
                                   const dbt::DBTInfo& dbt_info);

        void cluster_end() override;

        bool from_sub_cluster(const IdentityPtr& from_id) override;

        static SerializablePtr create_node_data();

        StatusFlag handle_disconnect_request(const IdentityPtr& from_id) override;

        class Broadcast : public EventProgress {
        public:
            explicit Broadcast(ProgressType type, uint32_t version, ProcessingRulesPtr rp);

            static StatusFlag create(ProcessingRulesPtr rp, Handle& handle);

            StatusFlag handle_event(const MetaData& meta, Variant& data, Handle& handle) override;

        protected:
            StatusFlag send_data(DataSet& dataset, Handle& handle) const;

            uint32_t _finish_count = 0;
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

            uint32_t _finish_count = 0;

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
            StatusFlag acquire_data(DataPtr& data, uint32_t rest_size, Handle& handle);

            ProgressEventsMI _self;

            DataSet _set;

            uint32_t _finish_size = 0;

        };

        class ReduceAgent : public Reduce, public EventProgressAgent {
        public:
            ReduceAgent(uint32_t version, ProcessingRulesPtr rp, ProgressEventsMI iter);

            static StatusFlag create(ProcessingRulesPtr rp, ProgressEventsMI other,
                                     Handle& handle, EventProgressAgent **agent_ptr);

            StatusFlag handle_event(const MetaData& meta, Variant& data, Handle& handle) override;


            StatusFlag proxy_event(const MetaData& meta, Variant& data, Handle& handle) override;

        private:
            StatusFlag close(DataSet& dataset, Handle& handle) const;

            ProgressEventsMI _other;

        };

    };

} // tdcf
