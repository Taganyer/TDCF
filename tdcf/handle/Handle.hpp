//
// Created by taganyer on 25-6-14.
//
#pragma once

#include <set>
#include <tdcf/handle/CommunicatorHandle.hpp>
#include <tdcf/handle/ProcessorHandle.hpp>

namespace tdcf {

    class Handle : public CommunicatorHandle, public ProcessorHandle {
    public:
        Handle(IdentityPtr ip, CommunicatorPtr cp, ProcessorPtr pp, IdentityPtr cluster) :
            CommunicatorHandle(std::move(cp)),
            ProcessorHandle(std::move(pp)),
            _self_id(std::move(ip)), _root_id(std::move(cluster)) {};

        Handle(IdentityPtr ip, CommunicatorPtr cp, ProcessorPtr pp):
            CommunicatorHandle(std::move(cp)),
            ProcessorHandle(std::move(pp)),
            _self_id(std::move(ip)) {};

        void set_cluster_size(uint32_t size) { _cluster_size = size; };

        const IdentityPtr& _self_identity() { return _self_id; };

        const IdentityPtr& root_identity() const { return _root_id; };

        uint32_t cluster_size() const { return _cluster_size; };

        uint32_t cluster_events() const { return _cluster_events; };

        uint32_t total_events() const { return progress_events.size(); };

    private:
        IdentityPtr _self_id;

        IdentityPtr _root_id;

        uint32_t _cluster_size = 0;

        uint32_t _cluster_events = 0;

        ProgressEventsMap progress_events;

    public:
        using IdentityList = std::set<IdentityPtr>;

        using IdentityIter = IdentityList::iterator;

        ProcessorAgentFactoryPtr agent_factory;

        IdentityList identities;

        ProgressEventsMI create_progress(EventProgressPtr&& progress);

        void destroy_progress(ProgressEventsMI iter);

        ProgressEventsMI find_progress(uint32_t version) {
            return progress_events.find(version);
        };

        bool check_progress(ProgressEventsMI iter) {
            return iter != progress_events.end();
        };

    };

}
