//
// Created by taganyer on 25-7-3.
//
#pragma once

#include <tdcf/frame/Processor.hpp>

namespace test {

    class Processor1 : public tdcf::Processor {
    public:
        explicit Processor1(uint32_t id) : _id(id) {};

        void acquire(tdcf::ProcessorEventMark mark, const tdcf::ProcessingRulesPtr& rule_ptr) override;

        void store(const tdcf::ProcessingRulesPtr& rule_ptr, const tdcf::DataPtr& data_ptr) override;

        void reduce(tdcf::ProcessorEventMark mark, const tdcf::ProcessingRulesPtr& rule_ptr,
                    const tdcf::DataSet& target) override;

        void scatter(tdcf::ProcessorEventMark mark, const tdcf::ProcessingRulesPtr& rule_ptr,
                     uint32_t scatter_size, const tdcf::DataSet& dataset) override;

        tdcf::OperationFlag get_events(EventQueue& queue) override;

    private:
        uint32_t _id;

        EventQueue _store;

    };

}
