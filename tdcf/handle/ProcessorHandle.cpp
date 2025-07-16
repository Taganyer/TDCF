//
// Created by taganyer on 25-6-29.
//

#include <tdcf/base/Errors.hpp>
#include <tdcf/handle/ProcessorHandle.hpp>
#include <utility>

using namespace tdcf;

ProcessorHandle::ProcessorHandle(ProcessorPtr ptr) : _processor(std::move(ptr)) {
    TDCF_CHECK_EXPR(_processor)
}

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
                                  const ProcessingRulesPtr& rule_ptr, DataSet target) {
    ProcessorEventMark mark = get_mark(iter);
    auto [i, success] = _process_delay.emplace(mark, std::pair(iter, meta));
    assert(success);
    _processor->reduce(mark, rule_ptr, std::move(target));
}

void ProcessorHandle::scatter_data(ProgressEventsMI iter, const MetaData& meta,
                                   const ProcessingRulesPtr& rule_ptr,
                                   uint32_t scatter_size, DataSet dataset) {
    ProcessorEventMark mark = get_mark(iter);
    auto [i, success] = _process_delay.emplace(mark, std::pair(iter, meta));
    assert(success);
    _processor->scatter(mark, rule_ptr, scatter_size, std::move(dataset));
}

void ProcessorHandle::create_processor_event(ProgressEventsMI iter,
                                             const MetaData& meta, SerializablePtr ptr) {
    _processed_queue.emplace(iter, meta, std::move(ptr));
}

void ProcessorHandle::create_processor_event(ProgressEventsMI iter,
                                             const MetaData& meta, DataSet dataset) {
    _processed_queue.emplace(iter, meta, std::move(dataset));
}

StatusFlag ProcessorHandle::get_processor_events() {
    OperationFlag flag = _processor->get_events(_data_queue);
    if (flag != OperationFlag::Success) {
        return flag == OperationFlag::Error ? StatusFlag::ProcessorGetEventsError :
                   StatusFlag::ProcessorGetEventsFurtherWaiting;
    }
    ProgressTask task;
    while (get_task(task)) {
        _processed_queue.emplace(std::move(task));
    }
    return StatusFlag::Success;
}

bool ProcessorHandle::get_progress_task(ProgressTask& task) {
    if (_processed_queue.empty()) return false;
    task = std::move(_processed_queue.front());
    _processed_queue.pop();
    return true;
}

ProcessorEventMark ProcessorHandle::get_mark(ProgressEventsMI iter) {
    return { iter->first, ++_version.version };
}

ProcessorHandle::ProgressTask::ProgressTask(ProgressEventsMI iter,
                                            const MetaData& meta, DataSet data) :
    iter(iter), meta(meta), result(std::move(data)) {}

ProcessorHandle::ProgressTask::ProgressTask(ProgressEventsMI iter,
                                            const MetaData& meta, SerializablePtr ptr) :
    iter(iter), meta(meta) {
    if (ptr && ptr->base_type() == (int) SerializableBaseType::Data) {
        result = std::static_pointer_cast<Data>(ptr);
    } else {
        result = std::move(ptr);
    }
}

bool ProcessorHandle::get_task(ProgressTask& task) {
    while (!_data_queue.empty()) {
        auto [type, mark, result] = std::move(_data_queue.front());
        _data_queue.pop();
        TDCF_CHECK_EXPR(!result.empty())
        auto iter = _process_delay.find(mark);
        if (iter == _process_delay.end()) continue;
        task = ProgressTask(iter->second.first, iter->second.second, std::move(result));
        _process_delay.erase(iter);
        return true;
    }
    return false;
}
