//
// Created by taganyer on 25-6-16.
//
#pragma once

#include <tdcf/frame/Data.hpp>

namespace tdcf {

    using StageNum = uint8_t;

    struct ClusterBroadcast {
        static constexpr StageNum acquire = 1;

        static constexpr StageNum send = 2;

        static constexpr StageNum finish_ack = 3;

    };

    struct NodeAgentBroadcast {
        static constexpr StageNum acquire = ClusterBroadcast::send;

        static constexpr StageNum send = 4;

        static constexpr StageNum finish = ClusterBroadcast::finish_ack;

        static constexpr StageNum finish_ack = 5;

    };

    struct AgentBroadcast {
        static constexpr StageNum acquire = NodeAgentBroadcast::send;

        static constexpr StageNum finish = NodeAgentBroadcast::finish_ack;

    };


}
