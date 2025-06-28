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

    enum class OperationFlag : uint8_t;

    struct ProcessorEventMark {
        Version version;
        uint32_t serial = 0;
    };

    struct ProcessorEvent {
        enum Type : uint8_t {
            Null,
            Acquire,
            Reduce,
            Scatter,
            Error,
        };

        Type type = Null;
        ProcessorEventMark mark;
        DataVariant result;

    };


    /// 如果需要进行异步处理，请对非 const& 的对象进行复制，因为框架不会持有该对象，同时要注意多线程问题。
    class Processor : NoCopy {
    public:
        using EventQueue = std::queue<ProcessorEvent>;

        Processor() = default;

        virtual ~Processor() = default;

        virtual void store(const ProcessingRulesPtr& rule_ptr, const DataPtr& data_ptr) = 0;

        virtual void acquire(ProcessorEventMark mark, const ProcessingRulesPtr& rule_ptr) = 0;

        virtual void reduce(ProcessorEventMark mark, const ProcessingRulesPtr& rule_ptr,
                            const DataSet& target) = 0;

        virtual void scatter(ProcessorEventMark mark, const ProcessingRulesPtr& rule_ptr,
                             unsigned scatter_size, const DataPtr& data_ptr) = 0;

        virtual OperationFlag get_events(EventQueue& queue) = 0;

    };

    using ProcessorPtr = std::shared_ptr<Processor>;

}
