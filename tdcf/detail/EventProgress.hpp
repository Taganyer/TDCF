//
// Created by taganyer on 25-5-26.
//
#pragma once

#include <memory>
#include <tdcf/frame/Data.hpp>
#include <tdcf/frame/Processor.hpp>

namespace tdcf {

    enum class ProgressType : uint8_t;

    class NodeInformation;

    using Variant = std::variant<SerializablePtr, DataPtr, DataSet>;

    struct EventProgress {
        explicit EventProgress(ProgressType type, ProcessingRulesPtr rule) :
            type(type), rule(std::move(rule)) {};

        virtual ~EventProgress() = default;

        virtual StatusFlag handle_event(const MetaData& meta, Variant& data, NodeInformation& info) = 0;

        virtual void handle_error(NodeInformation& info) = 0;

        ProgressType type;

        ProcessingRulesPtr rule;

    };

    using EventProgressPtr = std::unique_ptr<EventProgress>;

    using ProgressEventsMap = std::unordered_map<MetaData, EventProgressPtr>;

    using ProgressEventsMI = ProgressEventsMap::iterator;

    struct EventProgressAgent {
        EventProgressAgent() = default;

        virtual ~EventProgressAgent() = default;

        virtual StatusFlag proxy_event(const MetaData& meta, Variant& data, NodeInformation& info) = 0;

    };

    class ProcessorAgentFactory {
    public:
        ProcessorAgentFactory() = default;

        virtual ~ProcessorAgentFactory() = default;

        virtual StatusFlag broadcast(const ProcessingRulesPtr& rule, ProgressEventsMI iter,
                                     NodeInformation& info, EventProgressAgent **agent_ptr) = 0;

        virtual StatusFlag scatter(const ProcessingRulesPtr& rule, ProgressEventsMI iter,
                                   NodeInformation& info, EventProgressAgent **agent_ptr) = 0;

        virtual StatusFlag reduce(const ProcessingRulesPtr& rule, ProgressEventsMI iter,
                                  NodeInformation& info, EventProgressAgent **agent_ptr) = 0;

        virtual StatusFlag all_reduce(const ProcessingRulesPtr& rule, ProgressEventsMI iter,
                                      NodeInformation& info, EventProgressAgent **agent_ptr) = 0;

        virtual StatusFlag reduce_scatter(const ProcessingRulesPtr& rule, ProgressEventsMI iter,
                                          NodeInformation& info, EventProgressAgent **agent_ptr) = 0;

    };

    using ProcessorAgentFactoryPtr = std::unique_ptr<ProcessorAgentFactory>;

#define ProcessorAgentFactoryInherit(classname) \
    class classname : public ProcessorAgentFactory { \
    public: \
        StatusFlag broadcast(const ProcessingRulesPtr& rule, ProgressEventsMI iter, \
                             NodeInformation& info, EventProgressAgent **agent_ptr) override; \
        \
        StatusFlag scatter(const ProcessingRulesPtr& rule, ProgressEventsMI iter, \
                           NodeInformation& info, EventProgressAgent **agent_ptr) override; \
        \
        StatusFlag reduce(const ProcessingRulesPtr& rule, ProgressEventsMI iter, \
                          NodeInformation& info, EventProgressAgent **agent_ptr) override; \
        \
        StatusFlag all_reduce(const ProcessingRulesPtr& rule, ProgressEventsMI iter, \
                              NodeInformation& info, EventProgressAgent **agent_ptr) override; \
        \
        StatusFlag reduce_scatter(const ProcessingRulesPtr& rule, ProgressEventsMI iter, \
                                  NodeInformation& info, EventProgressAgent **agent_ptr) override; \
        \
    };

}
