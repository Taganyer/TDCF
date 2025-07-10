//
// Created by taganyer on 25-5-26.
//
#pragma once

#include <memory>
#include <unordered_map>
#include <tdcf/detail/StatusFlag.hpp>
#include <tdcf/frame/Data.hpp>
#include <tdcf/frame/Processor.hpp>

namespace tdcf {

    enum class ProgressType : uint8_t;

    class Handle;

    using Variant = std::variant<SerializablePtr, DataPtr, DataSet>;

    struct EventProgress {
        explicit EventProgress(OperationType o_type, ProgressType p_type,
                               uint32_t version, ProcessingRulesPtr rule) :
            version(version), operation_type(o_type),
            progress_type(p_type), rule(std::move(rule)) {};

        virtual ~EventProgress() = default;

        virtual StatusFlag handle_event(const MetaData& meta, Variant& data, Handle& handle) = 0;

        [[nodiscard]] MetaData create_meta() const {
            MetaData meta;
            meta.version = version;
            meta.operation_type = operation_type;
            meta.progress_type = progress_type;
            meta.serial = serial;
            return meta;
        };

        uint32_t version = 0;

        OperationType operation_type;

        ProgressType progress_type;

        uint32_t serial = 0;

        ProcessingRulesPtr rule;

    };

    using EventProgressPtr = std::unique_ptr<EventProgress>;

    using ProgressEventsMap = std::unordered_map<uint32_t, EventProgressPtr>;

    using ProgressEventsMI = ProgressEventsMap::iterator;

    struct EventProgressAgent {
        EventProgressAgent() = default;

        virtual ~EventProgressAgent() = default;

        virtual StatusFlag proxy_event(const MetaData& meta, Variant& data, Handle& handle) = 0;

    };

    class ProcessorAgentFactory {
    public:
        ProcessorAgentFactory() = default;

        virtual ~ProcessorAgentFactory() = default;

        virtual StatusFlag broadcast(const ProcessingRulesPtr& rule, ProgressEventsMI iter,
                                     Handle& handle, EventProgressAgent **agent_ptr) = 0;

        virtual StatusFlag scatter(const ProcessingRulesPtr& rule, ProgressEventsMI iter,
                                   Handle& handle, EventProgressAgent **agent_ptr) = 0;

        virtual StatusFlag reduce(const ProcessingRulesPtr& rule, ProgressEventsMI iter,
                                  Handle& handle, EventProgressAgent **agent_ptr) = 0;

        virtual StatusFlag all_reduce(const ProcessingRulesPtr& rule, ProgressEventsMI iter,
                                      Handle& handle, EventProgressAgent **agent_ptr) = 0;

        virtual StatusFlag reduce_scatter(const ProcessingRulesPtr& rule, ProgressEventsMI iter,
                                          Handle& handle, EventProgressAgent **agent_ptr) = 0;

    };

    using ProcessorAgentFactoryPtr = std::unique_ptr<ProcessorAgentFactory>;

#define ProcessorAgentFactoryInherit(classname) \
    class classname : public ProcessorAgentFactory { \
    public: \
        StatusFlag broadcast(const ProcessingRulesPtr& rule, ProgressEventsMI iter, \
                             Handle& handle, EventProgressAgent **agent_ptr) override; \
        \
        StatusFlag scatter(const ProcessingRulesPtr& rule, ProgressEventsMI iter, \
                           Handle& handle, EventProgressAgent **agent_ptr) override; \
        \
        StatusFlag reduce(const ProcessingRulesPtr& rule, ProgressEventsMI iter, \
                          Handle& handle, EventProgressAgent **agent_ptr) override; \
        \
        StatusFlag all_reduce(const ProcessingRulesPtr& rule, ProgressEventsMI iter, \
                              Handle& handle, EventProgressAgent **agent_ptr) override; \
        \
        StatusFlag reduce_scatter(const ProcessingRulesPtr& rule, ProgressEventsMI iter, \
                                  Handle& handle, EventProgressAgent **agent_ptr) override; \
        \
    };

}
