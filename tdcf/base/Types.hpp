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
        Broadcast,
        Scatter,
        Reduce,
        AllGather,
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
        static constexpr StageNum connect = 0;

        static constexpr StageNum start = 1;

        static constexpr StageNum disconnect = 1;

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

        static constexpr StageNum finish = ClusterBroadcast::finish_ack;

        static constexpr StageNum finish_ack = 7;

    };

    struct AgentBroadcast {
        static constexpr StageNum get_rule = NodeAgentBroadcast::send_rule;

        static constexpr StageNum get_data = NodeAgentBroadcast::send_data;

        static constexpr StageNum finish = NodeAgentBroadcast::finish_ack;

    };

}
