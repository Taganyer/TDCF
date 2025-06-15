//
// Created by taganyer on 25-5-26.
//
#pragma once

#include <memory>
#include <tdcf/frame/Data.hpp>
#include <tdcf/frame/Processor.hpp>

namespace tdcf {

    enum class EventType : uint8_t {
        Null,

        HTCBroadcast,
        HTCScatter,
        HTCReduce,
        HTCAllGather,
        HTCAllReduce,
        HTCReduceScatter,
        HTCAllToAll,

        CTNBroadcast,
        CTNScatter,
        CTNReduce,
        CTNAllGather,
        CTNAllReduce,
        CTNReduceScatter,
        CTNAllToAll,

        CTCBroadcast,
        CTCScatter,
        CTCReduce,
        CTCAllGather,
        CTCAllReduce,
        CTCReduceScatter,
        CTCAllToAll,
    };

    class NodeInformation;

    using Variant = std::variant<SerializablePtr, DataPtr, DataSet>;

    struct EventProgress {
        explicit EventProgress(EventType type, ProcessingRulesPtr rule) :
            type(type), rule(std::move(rule)) {};

        virtual ~EventProgress() = default;

        virtual StatusFlag handle_event(const MetaData& meta, Variant& data, NodeInformation& node_info) = 0;

        EventType type;

        ProcessingRulesPtr rule;

    };

    using EventProgressPtr = std::unique_ptr<EventProgress>;

    struct EventProgressAgent {
        EventProgressAgent() = default;

        virtual ~EventProgressAgent() = default;

        virtual StatusFlag store(NodeInformation& node_info) = 0;

        virtual StatusFlag access(NodeInformation& node_info) = 0;

    };

    struct ProcessorAgentFactory {
        ProcessorAgentFactory() = default;

        virtual ~ProcessorAgentFactory() = default;

        virtual StatusFlag broadcast(const ProcessingRulesPtr& rule, EventProgress *ptr,
                                     NodeInformation& node_info, EventProgressAgent **agent_ptr) = 0;

        virtual StatusFlag scatter(const ProcessingRulesPtr& rule, EventProgress *ptr,
                                   NodeInformation& node_info, EventProgressAgent **agent_ptr) = 0;

        virtual StatusFlag reduce(const ProcessingRulesPtr& rule, EventProgress *ptr,
                                  NodeInformation& node_info, EventProgressAgent **agent_ptr) = 0;

        virtual StatusFlag all_gather(const ProcessingRulesPtr& rule, EventProgress *ptr,
                                      NodeInformation& node_info, EventProgressAgent **agent_ptr) = 0;

        virtual StatusFlag all_reduce(const ProcessingRulesPtr& rule, EventProgress *ptr,
                                      NodeInformation& node_info, EventProgressAgent **agent_ptr) = 0;

        virtual StatusFlag reduce_scatter(const ProcessingRulesPtr& rule, EventProgress *ptr,
                                          NodeInformation& node_info) = 0;

        virtual StatusFlag all_to_all(const ProcessingRulesPtr& rule, EventProgress *ptr,
                                      NodeInformation& node_info, EventProgressAgent **agent_ptr) = 0;

    };

    using ProcessorAgentFactoryPtr = std::unique_ptr<ProcessorAgentFactory>;

}
