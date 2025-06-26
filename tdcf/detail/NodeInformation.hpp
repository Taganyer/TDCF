//
// Created by taganyer on 25-6-14.
//
#pragma once

#include <map>
#include <unordered_map>
#include <tdcf/detail/EventProgress.hpp>
#include <tdcf/frame/Communicator.hpp>

namespace tdcf {

    class NodeInformation {
    public:
        NodeInformation(IdentityPtr ip, CommunicatorPtr cp, ProcessorPtr pp, IdentityPtr cluster) :
            _id(std::move(ip)),
            _communicator(std::move(cp)),
            _processor(std::move(pp)),
            _root_id(std::move(cluster)) { assert(_id && _communicator && _processor); };

        NodeInformation(IdentityPtr ip, CommunicatorPtr cp, ProcessorPtr pp) :
            NodeInformation(std::move(ip), std::move(cp), std::move(pp), nullptr) {};

        void set_cluster_size(unsigned size) { _cluster_size = size; };

        void connect(const IdentityPtr& id) const;

        void accept(const IdentityPtr& id) const;

        void disconnect(const IdentityPtr& id) const;

        const IdentityPtr& id() const { return _id; };

        const IdentityPtr& root_id() const { return _root_id; };

        unsigned cluster_size() const { return _cluster_size; };

        Version get_version() { return _version++; };

    private:
        IdentityPtr _id;

        CommunicatorPtr _communicator;

        ProcessorPtr _processor;

        IdentityPtr _root_id;

        unsigned _cluster_size = 0;

        Version _version;

    public:
        struct ProgressTask {
            ProgressEventsMI iter;
            MetaData meta;
            Variant result;

            ProgressTask(ProgressEventsMI iter, const MetaData& meta, DataVariant data) :
                iter(iter), meta(meta) {
                if (data.index() == 0) {
                    result = std::move(std::get<DataPtr>(data));
                } else {
                    result = std::move(std::get<DataSet>(data));
                }
            };

            ProgressTask(ProgressEventsMI iter, const MetaData& meta, SerializablePtr ptr) :
                iter(iter), meta(meta), result(std::move(ptr)) {};

        };

        using IdentityList = std::vector<IdentityPtr>;

        ProcessorAgentFactoryPtr agent_factory;

        IdentityList identity_list;

        ProgressEventsMap progress_events;

    private:
        using SendDelayMQ = std::map<IdentityPtr, std::queue<std::pair<MetaData, SerializablePtr>>>;

        SendDelayMQ _message_delay;

    public:
        using MessageRQ = Communicator::EventQueue;

        MessageRQ message_queue;

        StatusFlag get_communicator_events() TDCF_NO_THROW;

        StatusFlag send_message(const IdentityPtr& id, const MetaData& meta, SerializablePtr message) TDCF_NO_THROW;

        void send_delay_message(const IdentityPtr& id) TDCF_NO_THROW;

        bool delayed_message(const IdentityPtr& id) TDCF_NO_THROW;

    private:
        using DataRQ = Processor::EventQueue;

        using ProgressDelayM = std::unordered_map<Version, std::pair<ProgressEventsMI, MetaData>>;

        DataRQ _data_queue;

        ProgressDelayM _process_delay;

        Version _data_version;

        Version get_data_version();

    public:
        using ProcessedQueue = std::queue<ProgressTask>;

        ProcessedQueue processed_queue;

        StatusFlag get_progress_tasks();

        void acquire_data(ProgressEventsMI iter, const MetaData& meta,
                                const ProcessingRulesPtr& rule_ptr) TDCF_THROW;

        void store_data(const ProcessingRulesPtr& rule_ptr, const DataPtr& data_ptr) const TDCF_THROW;

        void reduce_data(ProgressEventsMI iter, const MetaData& meta,
                               const ProcessingRulesPtr& rule_ptr, const DataSet& target) TDCF_THROW;

        void scatter_data(ProgressEventsMI iter, const MetaData& meta,
                                const ProcessingRulesPtr& rule_ptr,
                                unsigned scatter_size, const DataPtr& data_ptr) TDCF_THROW;

    };

}
