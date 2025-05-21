//
// Created by taganyer on 25-5-20.
//
#pragma once

#include <vector>

#include "Data.hpp"
#include "Serializable.hpp"

namespace tdcf {

    class ProcessingRules : public Serializable {
    public:
        ProcessingRules() = default;

        ~ProcessingRules() override = default;

    };

    class ProcessorFlag {};

    class Processor {
    public:
        using DataSet = std::vector<DataPtr>;

        Processor() = default;

        virtual ~Processor() = default;

        virtual ProcessorFlag acquire(const MetaData& data, Data& buffer) = 0;

        virtual ProcessorFlag store(const Data& data) = 0;

        virtual ProcessorFlag filtering(const ProcessingRules& rule,
                                        const Data& data, Data& buffer) = 0;

        virtual ProcessorFlag reduce(const ProcessingRules& rule,
                                     const DataSet& target, Data& buffer) = 0;

        virtual ProcessorFlag scatter(const ProcessingRules& rule,
                                      const DataSet& target, DataSet& buffer) = 0;

    };

    using ProcessorPtr = std::shared_ptr<Processor>;

}
