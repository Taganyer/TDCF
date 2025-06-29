//
// Created by taganyer on 25-6-29.
//

#include <tdcf/base/Errors.hpp>
#include <tdcf/handle/ProcessorHandle.hpp>

using namespace tdcf;

void ProcessorHandle::acquire_data(ProgressEventsMI iter, const MetaData& meta,
                                   const ProcessingRulesPtr& rule_ptr) {
    ProcessorEventMark mark = get_mark(iter);
    auto [i, success] = _process_delay.emplace(mark, std::pair(iter, meta));
    assert(success);
    _processor->acquire(mark, rule_ptr);
}

void ProcessorHandle::store_data(const ProcessingRulesPtr& rule_ptr, const DataPtr& data_ptr) const {
    _processor->store(rule_ptr, data_ptr);
}

void ProcessorHandle::reduce_data(ProgressEventsMI iter, const MetaData& meta,
                                  const ProcessingRulesPtr& rule_ptr, const DataSet& target) {
    ProcessorEventMark mark = get_mark(iter);
    auto [i, success] = _process_delay.emplace(mark, std::pair(iter, meta));
    assert(success);
    _processor->reduce(mark, rule_ptr, target);
}

void ProcessorHandle::scatter_data(ProgressEventsMI iter, const MetaData& meta,
                                   const ProcessingRulesPtr& rule_ptr,
                                   uint32_t scatter_size, const DataPtr& data_ptr) {
    ProcessorEventMark mark = get_mark(iter);
    auto [i, success] = _process_delay.emplace(mark, std::pair(iter, meta));
    assert(success);
    _processor->scatter(mark, rule_ptr, scatter_size, data_ptr);
}

StatusFlag ProcessorHandle::get_processor_events() {
    OperationFlag flag = _processor->get_events(_data_queue);
    if (flag != OperationFlag::Success) {
        return flag == OperationFlag::Error ? StatusFlag::ProcessorGetEventsError :
                   StatusFlag::ProcessorGetEventsFurtherWaiting;
    }
    return StatusFlag::Success;
}

bool ProcessorHandle::get_task(ProgressTask& task) {
    while (!_data_queue.empty()) {
        auto [type, mark, result] = std::move(_data_queue.front());
        _data_queue.pop();
        auto iter = _process_delay.find(mark);
        if (iter == _process_delay.end()) continue;
        MetaData meta = iter->second.second;
        if (type == ProcessorEvent::Error) {
            meta.operation_type = OperationType::Error;
        }
        task = std::move(ProgressTask(iter->second.first, meta, std::move(result)));
        _process_delay.erase(iter);
        return true;
    }
    return false;
}

uint32_t ProcessorHandle::processor_event_size() const {
    return _data_queue.size();
}

ProcessorEventMark ProcessorHandle::get_mark(ProgressEventsMI iter) {
    return { iter->first.version, ++_version.version };
}

ProcessorHandle::ProgressTask::ProgressTask(ProgressEventsMI iter,
                                            const MetaData& meta, DataVariant data) :
    iter(iter), meta(meta) {
    if (data.index() == 0) {
        result = std::move(std::get<DataPtr>(data));
    } else {
        result = std::move(std::get<DataSet>(data));
    }
}
