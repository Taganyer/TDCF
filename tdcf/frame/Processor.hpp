//
// Created by taganyer on 25-5-20.
//
#pragma once

#include <vector>
#include <tdcf/base/NoCopy.hpp>
#include <tdcf/frame/Data.hpp>

namespace tdcf {

    class ProcessingRules : public Message {
    public:
        ProcessingRules() = default;

        [[nodiscard]] SerializableType base_type() const final {
            return static_cast<SerializableType>(SerializableBaseTypes::ProcessingRules);
        };

    };

    using ProcessingRulesPtr = std::shared_ptr<ProcessingRules>;

    class Processor : NoCopy {
    public:
        using DataSet = std::vector<DataPtr>;

        Processor() = default;

        virtual ~Processor() = default;

        virtual StatusFlag acquire(const ProcessingRulesPtr& rule_ptr,
                                   DataPtr& buffer_ptr) = 0;

        virtual StatusFlag store(const ProcessingRulesPtr& rule_ptr,
                                 const DataPtr& data_ptr) = 0;

        virtual StatusFlag reduce(const ProcessingRulesPtr& rule_ptr,
                                  const DataSet& target, DataPtr& buffer_ptr) = 0;

        virtual StatusFlag scatter(const ProcessingRulesPtr& rule_ptr, unsigned scatter_size,
                                   const DataPtr& data_ptr, DataSet& buffer) = 0;

    };

    using ProcessorPtr = std::shared_ptr<Processor>;

}
