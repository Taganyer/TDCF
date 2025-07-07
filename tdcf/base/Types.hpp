//
// Created by taganyer on 25-6-19.
//
#pragma once

#include <cstdint>
#include <tdcf/base/Marcos.hpp>

namespace tdcf {

#define OPERATION_TYPE_ITEM(MOD) \
    MOD(Null), \
    MOD(AgentCreate), \
    MOD(Init), \
    MOD(Close), \
    MOD(Broadcast), \
    MOD(Scatter), \
    MOD(Reduce), \
    MOD(AllReduce), \
    MOD(ReduceScatter),

    enum class OperationType : uint8_t {
        OPERATION_TYPE_ITEM(TDCF_ENUM_MOD)
    };

    constexpr const char* operation_type_name(OperationType type) {
        constexpr const char *item_names[] = {
            OPERATION_TYPE_ITEM(TDCF_NAME_MOD)
        };
        return item_names[static_cast<int>(type)];
    };

#undef OPERATION_TYPE_ITEM

#define SERIALIZABLE_BASE_TYPE(MOD) \
    MOD(Null), \
    MOD(Message), \
    MOD(Identity), \
    MOD(NodeAgent), \
    MOD(Data), \
    MOD(ProcessingRules),

    enum class SerializableBaseType : uint8_t {
        SERIALIZABLE_BASE_TYPE(TDCF_ENUM_MOD)
    };

    constexpr const char* serializable_base_type_name(SerializableBaseType type) {
        constexpr const char *item_names[] = {
            SERIALIZABLE_BASE_TYPE(TDCF_NAME_MOD)
        };
        return item_names[static_cast<int>(type)];
    };

#undef SERIALIZABLE_BASE_TYPE

    enum class LinkMark : uint8_t {
        Null,
        Create,
        Info,
    };

    enum class ProgressType : uint8_t {
        Null,
        Root,
        Node,
        NodeRoot,
    };

    enum class OperationFlag : uint8_t {
        Success,
        FurtherWaiting,
        Error,
    };

    using StageNum = uint8_t;

    struct ClusterType {
        static constexpr StageNum star = 1;

        static constexpr StageNum ring = 2;

    };

    struct Star {
        static constexpr StageNum start = 1;

        static constexpr StageNum close = 2;

    };

    struct Ring {
        static constexpr StageNum start = 1;

        static constexpr StageNum close = 2;

    };

    struct ClusterBroadcast {
        static constexpr StageNum acquire_data = 1;

        static constexpr StageNum send_rule = 2;

        static constexpr StageNum send_data = 3;

        static constexpr StageNum finish_ack = 4;

    };

    struct NodeAgentBroadcast {
        static constexpr StageNum get_rule = ClusterBroadcast::send_rule;

        static constexpr StageNum send_rule = 5;

        static constexpr StageNum get_data = ClusterBroadcast::send_data;

        static constexpr StageNum send_data = 6;

        static constexpr StageNum finish_ack = 7;

        static constexpr StageNum finish = ClusterBroadcast::finish_ack;

    };

    struct AgentBroadcast {
        static constexpr StageNum get_data = NodeAgentBroadcast::send_data;

        static constexpr StageNum finish_ack = ClusterBroadcast::finish_ack;

        static constexpr StageNum finish = NodeAgentBroadcast::finish_ack;

    };

    struct ClusterScatter {
        static constexpr StageNum acquire_data = 1;

        static constexpr StageNum send_rule = 2;

        static constexpr StageNum scatter_data = 3;

        static constexpr StageNum send_data = 4;

        static constexpr StageNum finish_ack = 5;

    };

    struct NodeAgentScatter {
        static constexpr StageNum get_rule = ClusterScatter::send_rule;

        static constexpr StageNum send_rule = 6;

        static constexpr StageNum get_data = ClusterScatter::send_data;

        static constexpr StageNum scatter_data = 7;

        static constexpr StageNum send_data = 8;

        static constexpr StageNum finish_ack = 9;

        static constexpr StageNum finish = ClusterScatter::finish_ack;

    };

    struct AgentScatter {
        static constexpr StageNum send_rule = ClusterScatter::send_rule;

        static constexpr StageNum get_data = NodeAgentScatter::send_data;

        static constexpr StageNum finish_ack = ClusterScatter::finish_ack;

        static constexpr StageNum finish = NodeAgentScatter::finish_ack;

    };

    struct ClusterReduce {
        static constexpr StageNum acquire_data = 1;

        static constexpr StageNum send_rule = 2;

        static constexpr StageNum reduce_data = 3;

    };

    struct NodeAgentReduce {
        static constexpr StageNum get_rule = ClusterReduce::send_rule;

        static constexpr StageNum send_rule = 4;

        static constexpr StageNum acquire_data = 5;

        static constexpr StageNum reduce_data = 6;

        static constexpr StageNum send_data = ClusterReduce::acquire_data;

    };

    struct AgentReduce {
        static constexpr StageNum send_rule = ClusterReduce::send_rule;

        static constexpr StageNum acquire_data = ClusterReduce::acquire_data;

        static constexpr StageNum reduce_data = ClusterReduce::reduce_data;

        static constexpr StageNum send_data = NodeAgentReduce::acquire_data;

    };

    struct ClusterAllReduce {
        static constexpr StageNum acquire_data = 1;

        static constexpr StageNum send_rule = 2;

        static constexpr StageNum reduce_data = 3;

        static constexpr StageNum send_data = 4;

        static constexpr StageNum finish_ack = 5;

    };

    struct NodeAgentAllReduce {
        static constexpr StageNum get_rule = ClusterAllReduce::send_rule;

        static constexpr StageNum acquire_data1 = 6;

        static constexpr StageNum send_data1 = ClusterAllReduce::acquire_data;

        static constexpr StageNum acquire_data2 = ClusterAllReduce::send_data;

        static constexpr StageNum send_data2 = 7;

        static constexpr StageNum finish_ack = 8;

        static constexpr StageNum finish = ClusterAllReduce::finish_ack;

    };

    struct AgentAllReduce {
        static constexpr StageNum acquire_data1 = ClusterAllReduce::acquire_data;

        static constexpr StageNum send_rule = ClusterAllReduce::send_rule;

        static constexpr StageNum reduce_data = ClusterAllReduce::reduce_data;

        static constexpr StageNum send_data = NodeAgentAllReduce::acquire_data1;

        static constexpr StageNum acquire_data2 = NodeAgentAllReduce::send_data2;

        static constexpr StageNum finish_ack = ClusterAllReduce::finish_ack;

        static constexpr StageNum finish = NodeAgentAllReduce::finish_ack;

    };

    struct ClusterReduceScatter {
        static constexpr StageNum acquire_data = 1;

        static constexpr StageNum send_rule = 2;

        static constexpr StageNum reduce_data = 3;

        static constexpr StageNum scatter_data = 4;

        static constexpr StageNum send_data = 5;

        static constexpr StageNum finish_ack = 6;

    };

    struct NodeAgentReduceScatter {
        static constexpr StageNum get_rule = ClusterReduceScatter::send_rule;

        static constexpr StageNum acquire_data1 = 7;

        static constexpr StageNum send_data1 = ClusterReduceScatter::acquire_data;

        static constexpr StageNum acquire_data2 = ClusterReduceScatter::send_data;

        static constexpr StageNum send_data2 = 8;

        static constexpr StageNum finish_ack = 9;

        static constexpr StageNum finish = ClusterReduceScatter::finish_ack;

    };

    struct AgentReduceScatter {
        static constexpr StageNum acquire_data1 = ClusterReduceScatter::acquire_data;

        static constexpr StageNum send_rule = ClusterReduceScatter::send_rule;

        static constexpr StageNum reduce_data = ClusterReduceScatter::reduce_data;

        static constexpr StageNum send_data1 = NodeAgentReduceScatter::acquire_data1;

        static constexpr StageNum acquire_data2 = NodeAgentReduceScatter::send_data2;

        static constexpr StageNum scatter_data = ClusterReduceScatter::scatter_data;

        static constexpr StageNum send_data2 = ClusterReduceScatter::send_data;

        static constexpr StageNum finish_ack = ClusterReduceScatter::finish_ack;

        static constexpr StageNum finish = NodeAgentReduceScatter::finish_ack;

    };

}
