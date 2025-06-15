//
// Created by taganyer on 25-6-14.
//
#pragma once

#include <map>
#include <list>
#include <unordered_map>

#include <tdcf/detail/EventProgress.hpp>
#include <tdcf/frame/Communicator.hpp>

namespace tdcf {

    class NodeInformation {
    public:
        IdentityPtr id;

        CommunicatorPtr communicator;

        ProcessorPtr processor;

        IdentityPtr root_id;

        NodeInformation() = default;

        NodeInformation(IdentityPtr ip, CommunicatorPtr cp, ProcessorPtr pp, IdentityPtr cluster) :
            id(std::move(ip)),
            communicator(std::move(cp)),
            processor(std::move(pp)),
            root_id(std::move(cluster)) {};

        NodeInformation(IdentityPtr ip, CommunicatorPtr cp, ProcessorPtr pp) :
            NodeInformation(std::move(ip), std::move(cp), std::move(pp), nullptr) {};

        [[nodiscard]] bool check() const {
            return id && communicator && processor;
        };

        using IdentityList = std::vector<IdentityPtr>;

        using ProgressEventsM = std::unordered_map<MetaData, EventProgressPtr>;

        using ProgressEventsMI = ProgressEventsM::iterator;

        struct ProgressTask {
            ProgressEventsMI iter;
            MetaData meta;
            Variant result;

            ProgressTask(ProgressEventsMI iter, const MetaData& meta, DataVariant result) :
                iter(iter), meta(meta),
                result(result.index() ? std::move(std::get<DataPtr>(result)) :
                           std::move(std::get<DataSet>(result))) {};

            ProgressTask(ProgressEventsMI iter, const MetaData& meta, SerializablePtr ptr) :
                iter(iter), meta(meta), result(std::move(ptr)) {};

        };

        Version progress_events_version;

        ProcessorAgentFactoryPtr agent_factory;

        IdentityList identity_list;

        ProgressEventsM progress_events;

    private:
        using SendDelayMQ = std::map<IdentityPtr, std::queue<std::pair<MetaData, SerializablePtr>>>;

        SendDelayMQ message_delay;

    public:
        using MessageRQ = Communicator::EventQueue;

        MessageRQ message_queue;

        StatusFlag get_communicator_event();

        StatusFlag send_message(const IdentityPtr& id, const MetaData& meta, SerializablePtr message);

        StatusFlag send_delay_message(const IdentityPtr& id);

    private:
        using DataRQ = Processor::EventQueue;

        using ProgressDelayM = std::unordered_map<Version, std::pair<ProgressEventsMI, MetaData>>;

        DataRQ data_queue;

        ProgressDelayM process_delay;

        Version data_version;

    public:
        using ProcessedQueue = std::queue<ProgressTask>;

        ProcessedQueue processed_queue;

        StatusFlag get_processor_data();

        StatusFlag acquire_data(ProgressEventsMI iter, const MetaData& meta,
                                const ProcessingRulesPtr& rule_ptr);

        StatusFlag store_data(const ProcessingRulesPtr& rule_ptr, const DataPtr& data_ptr);

        StatusFlag reduce_data(ProgressEventsMI iter, const MetaData& meta,
                               const ProcessingRulesPtr& rule_ptr, const DataSet& target);

        StatusFlag scatter_data(ProgressEventsMI iter, const MetaData& meta,
                                const ProcessingRulesPtr& rule_ptr,
                                unsigned scatter_size, const DataPtr& data_ptr);

    };

}
