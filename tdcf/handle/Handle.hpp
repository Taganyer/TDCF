//
// Created by taganyer on 25-6-14.
//
#pragma once

#include <tdcf/handle/CommunicatorHandle.hpp>
#include <tdcf/handle/ProcessorHandle.hpp>

namespace tdcf {

    class Handle : public CommunicatorHandle, public ProcessorHandle {
    public:
        Handle(IdentityPtr ip, CommunicatorPtr cp, ProcessorPtr pp, IdentityPtr cluster) :
            CommunicatorHandle(std::move(ip), std::move(cp)),
            ProcessorHandle(std::move(pp)),
            _root_id(std::move(cluster)) {};

        Handle(IdentityPtr ip, CommunicatorPtr cp, ProcessorPtr pp):
            CommunicatorHandle(std::move(ip), std::move(cp)),
            ProcessorHandle(std::move(pp)) {};

        void set_cluster_size(uint32_t size) { _cluster_size = size; };

        const IdentityPtr& root_identity() const { return _root_id; };

        void set_root_serial(uint32_t serial) { _root_serial = serial; };

        uint32_t root_serial() const { return _root_serial; };

        uint32_t cluster_size() const { return _cluster_size; };

    private:
        IdentityPtr _root_id;

        uint32_t _root_serial = -1;

        uint32_t _cluster_size = 0;

    public:
        ProcessorAgentFactoryPtr agent_factory;

        ProgressEventsMap progress_events;

    };

}
