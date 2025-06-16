//
// Created by taganyer on 25-6-14.
//

#include <tdcf/base/Errors.hpp>
#include <tdcf/detail/NodeInformation.hpp>

using namespace tdcf;

StatusFlag NodeInformation::get_communicator_events() {
    return communicator->get_events(message_queue);
}

StatusFlag NodeInformation::send_message(const IdentityPtr& id, const MetaData& meta, SerializablePtr message) {
    if (message_delay[id].empty()) {
        StatusFlag flag = communicator->send_message(id, Message(meta), message);
        if (flag != StatusFlag::FurtherWaiting) return flag;
    }
    message_delay[id].emplace(meta, std::move(message));
    return StatusFlag::Success;
}

StatusFlag NodeInformation::send_delay_message(const IdentityPtr& id) {
    auto& q = message_delay[id];
    while (!q.empty()) {
        StatusFlag flag = communicator->send_message(id, Message(q.front().first), q.front().second);
        if (flag == StatusFlag::FurtherWaiting) break;
        if (flag != StatusFlag::Success) return flag;
        q.pop();
    }
    return StatusFlag::Success;
}

StatusFlag NodeInformation::get_progress_tasks() {
    StatusFlag flag = processor->get_events(data_queue);
    if (flag != StatusFlag::Success) return flag;
    while (!data_queue.empty()) {
        auto event = std::move(data_queue.front());
        data_queue.pop();
        auto iter = process_delay.find(event.version);
        assert(iter != process_delay.end());
        processed_queue.emplace(iter->second.first, iter->second.second, std::move(event.result));
    }
    return StatusFlag::Success;
}

StatusFlag NodeInformation::acquire_data(ProgressEventsMI iter, const MetaData& meta,
                                         const ProcessingRulesPtr& rule_ptr) {
    auto [i, success] = process_delay.emplace(++data_version, std::pair(iter, meta));
    TDCF_CHECK_EXPR(success)
    StatusFlag flag = processor->acquire(data_version, rule_ptr);
    if (flag != StatusFlag::Success) process_delay.erase(i);
    return flag;
}

StatusFlag NodeInformation::store_data(const ProcessingRulesPtr& rule_ptr, const DataPtr& data_ptr) {
    return processor->store(rule_ptr, data_ptr);
}

StatusFlag NodeInformation::reduce_data(ProgressEventsMI iter, const MetaData& meta,
                                        const ProcessingRulesPtr& rule_ptr, const DataSet& target) {
    auto [i, success] = process_delay.emplace(++data_version, std::pair(iter, meta));
    TDCF_CHECK_EXPR(success)
    return processor->reduce(data_version, rule_ptr, target);
}

StatusFlag NodeInformation::scatter_data(ProgressEventsMI iter, const MetaData& meta,
                                         const ProcessingRulesPtr& rule_ptr,
                                         unsigned scatter_size, const DataPtr& data_ptr) {
    auto [i, success] = process_delay.emplace(++data_version, std::pair(iter, meta));
    TDCF_CHECK_EXPR(success)
    return processor->scatter(data_version, rule_ptr, scatter_size, data_ptr);
}
