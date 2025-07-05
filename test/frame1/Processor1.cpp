//
// Created by taganyer on 25-7-4.
//

#include <test/frame1/Data1.hpp>
#include <test/frame1/ProcessingRules1.hpp>
#include <test/frame1/Processor1.hpp>

using namespace test;

using namespace tdcf;


void Processor1::acquire(ProcessorEventMark mark, const ProcessingRulesPtr& rule_ptr) {
    auto& rule = static_cast<ProcessingRules1&>(*rule_ptr);
    T_INFO << "Processor " << _id << " require event " << rule.id();
    _store.emplace(ProcessorEvent { ProcessorEvent::Acquire, mark, std::make_shared<Data1>(rule.id()) });
}

void Processor1::store(const ProcessingRulesPtr& rule_ptr, const DataPtr& data_ptr) {
    auto& rule = static_cast<ProcessingRules1&>(*rule_ptr);
    auto& data = static_cast<Data1&>(*data_ptr);
    T_INFO << "Processor " << _id << " store event " << rule.id()
            << " data src " << data.src
            << " data size " << data.data.size();
}

void Processor1::reduce(ProcessorEventMark mark, const ProcessingRulesPtr& rule_ptr,
                        const DataSet& target) {
    auto& rule = static_cast<ProcessingRules1&>(*rule_ptr);
    auto data = std::make_shared<Data1>(_id);
    for (auto& d : target) {
        auto& d1 = static_cast<Data1&>(*d);
        assert(!d1.data.empty());
        data->data.insert(data->data.end(), d1.data.begin(), d1.data.end());
    }
    T_INFO << "Processor " << _id << " reduce event " << rule.id()
            << " dataset size is " << target.size()
            << " total size " << data->data.size();
    _store.emplace(ProcessorEvent { ProcessorEvent::Reduce, mark, std::move(data) });
}

void Processor1::scatter(ProcessorEventMark mark, const ProcessingRulesPtr& rule_ptr,
                         uint32_t scatter_size, const DataPtr& data_ptr) {
    auto& rule = static_cast<ProcessingRules1&>(*rule_ptr);
    DataSet set(scatter_size, data_ptr);
    T_INFO << "Processor " << _id << " scatter event " << rule.id()
            << " scatter size " << scatter_size;
    _store.emplace(ProcessorEvent { ProcessorEvent::Reduce, mark, std::move(set) });
}

OperationFlag Processor1::get_events(EventQueue& queue) {
    if (_store.empty()) return OperationFlag::FurtherWaiting;
    while (!_store.empty()) {
        queue.emplace(std::move(_store.front()));
        _store.pop();
    }
    return OperationFlag::Success;
}
