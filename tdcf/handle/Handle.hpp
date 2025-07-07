//
// Created by taganyer on 25-6-14.
//
#pragma once

#include <tdcf/handle/CommunicatorHandle.hpp>
#include <tdcf/handle/ProcessorHandle.hpp>

namespace tdcf {

    class Handle : public CommunicatorHandle, public ProcessorHandle {
    public:
        Handle(IdentityPtr ip, CommunicatorPtr cp, ProcessorPtr pp, IdentityPtr cluster);

        Handle(IdentityPtr ip, CommunicatorPtr cp, ProcessorPtr pp);

        void set_superior_identity(IdentityPtr superior_identity) {
            _superior_id = std::move(superior_identity);
        };

        const IdentityPtr& self_identity() { return _self_id; };

        uint32_t cluster_events() const { return _cluster_events; };

        uint32_t total_events() const { return progress_events.size(); };

    private:
        IdentityPtr _self_id;

        IdentityPtr _superior_id;

        uint32_t _cluster_events = 0;

        ProgressEventsMap progress_events;

        std::shared_ptr<void> _agent_extra_data = nullptr;

        std::shared_ptr<void> _cluster_extra_data = nullptr;

    public:
        ProcessorAgentFactoryPtr agent_factory;

        ProgressEventsMI create_progress(EventProgressPtr&& progress);

        void destroy_progress(ProgressEventsMI iter);

        ProgressEventsMI find_progress(uint32_t version) {
            return progress_events.find(version);
        };

        bool check_progress(ProgressEventsMI iter) {
            return iter != progress_events.end();
        };

        template <typename Class, typename... Args>
        void create_agent_data(Args&&... args) {
            _agent_extra_data = std::make_shared<Class>(std::forward<Args>(args)...);
        };

        template <typename Class, typename... Args>
        void create_cluster_data(Args&&... args) {
            _cluster_extra_data = std::make_shared<Class>(std::forward<Args>(args)...);
        };

        void destroy_agent_data() {
            _agent_extra_data = nullptr;
        };

        void destroy_cluster_data() {
            _cluster_extra_data = nullptr;
        };

        bool has_agent_data() const { return _agent_extra_data != nullptr; };

        bool has_cluster_data() const { return _cluster_extra_data != nullptr; };

        template <typename T>
        T& agent_data() {
            return *static_cast<T *>(_agent_extra_data.get());
        };

        template <typename T>
        T& cluster_data() {
            return *static_cast<T *>(_cluster_extra_data.get());
        };

    };

}
