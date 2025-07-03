//
// Created by taganyer on 25-7-3.
//
#pragma once

#include <tdcf/frame/Processor.hpp>

namespace test {

    class Processor1 : public tdcf::Processor {
    public:
        void acquire(tdcf::ProcessorEventMark mark, const tdcf::ProcessingRulesPtr& rule_ptr) override;

        void store(const tdcf::ProcessingRulesPtr& rule_ptr, const tdcf::DataPtr& data_ptr) override;

        void reduce(tdcf::ProcessorEventMark mark, const tdcf::ProcessingRulesPtr& rule_ptr,
                    const tdcf::DataSet& target) override;

        void scatter(tdcf::ProcessorEventMark mark, const tdcf::ProcessingRulesPtr& rule_ptr,
                     uint32_t scatter_size, const tdcf::DataPtr& data_ptr) override;

        tdcf::OperationFlag get_events(EventQueue& queue) override;

    };

}
