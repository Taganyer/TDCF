//
// Created by taganyer on 25-6-19.
//
#pragma once

#include <cstdint>

namespace tdcf {

    enum class OperationType : uint8_t {
        Null,
        Init,
        AgentCreate,
        Close,
        Broadcast,
        Scatter,
        Reduce,
        AllReduce,
        ReduceScatter,
        AllToAll,
    };

    enum class SerializableBaseTypes : uint8_t {
        Null,
        Message,
        Identity,
        NodeAgent,
        Data,
        ProcessingRules,
    };

    enum class ProgressType : uint8_t {
        Null,
        Root,
        Node,
        NodeRoot,
    };

    using StageNum = uint8_t;

    struct ClusterType {
        static constexpr StageNum star = 1;

    };

    struct Star {
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

}
