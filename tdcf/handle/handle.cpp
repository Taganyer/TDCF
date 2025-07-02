//
// Created by taganyer on 25-7-2.
//

#include <tdcf/base/Errors.hpp>
#include <tdcf/handle/Handle.hpp>

using namespace tdcf;


ProgressEventsMI Handle::create_progress(EventProgressPtr&& progress) {
    uint32_t version = progress->version;
    ProgressType type = progress->progress_type;
    auto [iter, success] = progress_events.emplace(version, std::move(progress));
    TDCF_CHECK_EXPR(success)
    if (type == ProgressType::Root) ++_cluster_events;
    return iter;
}

void Handle::destroy_progress(ProgressEventsMI iter) {
    assert(iter != progress_events.end());
    if (iter->second->progress_type == ProgressType::Root) {
        assert(_cluster_events);
        --_cluster_events;
    }
    close_conversation(iter->first);
    progress_events.erase(iter);
}
