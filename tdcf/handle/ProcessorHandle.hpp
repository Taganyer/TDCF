//
// Created by taganyer on 25-6-29.
//
#pragma once

#include <map>
#include <tdcf/base/Version.hpp>
#include <tdcf/detail/EventProgress.hpp>
#include <tdcf/detail/StatusFlag.hpp>
#include <tdcf/frame/Processor.hpp>

namespace tdcf {

    class ProcessorHandle {
    public:
        struct ProgressTask;

        explicit ProcessorHandle(ProcessorPtr ptr);

        void acquire_data(ProgressEventsMI iter, const MetaData& meta,
                          const ProcessingRulesPtr& rule_ptr);

        void store_data(const ProcessingRulesPtr& rule_ptr, const DataPtr& data_ptr) const;

        void reduce_data(ProgressEventsMI iter, const MetaData& meta,
                         const ProcessingRulesPtr& rule_ptr, const DataSet& target);

        void scatter_data(ProgressEventsMI iter, const MetaData& meta,
                          const ProcessingRulesPtr& rule_ptr,
                          uint32_t scatter_size, const DataPtr& data_ptr);

        void create_processor_event(ProgressEventsMI iter, const MetaData& meta, SerializablePtr ptr);

        StatusFlag get_processor_events();

        bool get_progress_task(ProgressTask& task);

    private:
        struct Cmp {
            bool operator()(ProcessorEventMark lhs, ProcessorEventMark rhs) const {
                if (lhs.version != rhs.version) return lhs.version < rhs.version;
                return lhs.serial < rhs.serial;
            };
        };

        ProcessorPtr _processor;

        Version _version;

        using DataRQ = Processor::EventQueue;

        DataRQ _data_queue;

        using ProgressDelayM = std::map<ProcessorEventMark, std::pair<ProgressEventsMI, MetaData>, Cmp>;

        ProgressDelayM _process_delay;

        ProcessorEventMark get_mark(ProgressEventsMI iter);

        bool get_task(ProgressTask& task);

    public:
        struct ProgressTask {
            ProgressEventsMI iter {};
            MetaData meta;
            Variant result;

            ProgressTask() = default;

            ProgressTask(ProgressEventsMI iter, const MetaData& meta, DataVariant data);

            ProgressTask(ProgressEventsMI iter, const MetaData& meta, SerializablePtr ptr);

        };

    private:
        using ProcessedQueue = std::queue<ProgressTask>;

        ProcessedQueue _processed_queue;

    };

}
