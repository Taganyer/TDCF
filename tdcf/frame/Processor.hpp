//
// Created by taganyer on 25-5-20.
//
#pragma once

#include <queue>
#include <variant>
#include <vector>
#include <tdcf/base/NoCopy.hpp>
#include <tdcf/frame/ProcessingRules.hpp>

namespace tdcf {

    using DataSet = std::vector<DataPtr>;

    using DataVariant = std::variant<DataPtr, DataSet>;

    struct ProcessorEvent {
        enum Type {
            Null,
            Acquire,
            Reduce,
            Scatter,
            Merge,
        };

        Type type = Null;
        Version version;

        DataVariant result;
    };


    class Processor : NoCopy {
    public:
        using EventQueue = std::queue<ProcessorEvent>;

        Processor() = default;

        virtual ~Processor() = default;

        virtual StatusFlag store(const ProcessingRulesPtr& rule_ptr,
                                 const DataPtr& data_ptr) = 0;

        virtual StatusFlag acquire(Version v, const ProcessingRulesPtr& rule_ptr) = 0;

        virtual StatusFlag reduce(Version v, const ProcessingRulesPtr& rule_ptr,
                                  const DataSet& target) = 0;

        virtual StatusFlag scatter(Version v, const ProcessingRulesPtr& rule_ptr,
                                   unsigned scatter_size, const DataPtr& data_ptr) = 0;

        virtual StatusFlag merge(Version v, const ProcessingRulesPtr& rule_ptr,
                                 const DataSet& target) = 0;

        virtual StatusFlag get_events(EventQueue& queue) = 0;

    };

    using ProcessorPtr = std::shared_ptr<Processor>;

}
