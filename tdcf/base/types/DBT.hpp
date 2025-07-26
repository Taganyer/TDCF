//
// Created by taganyer on 25-7-16.
//
#pragma once

#include <tdcf/base/Types.hpp>

namespace tdcf::dbt {

    struct C_Broadcast {
        static constexpr StageNum acquire_data = 1;

        static constexpr StageNum send_rule = 2;

        static constexpr StageNum send_data = 3;

        static constexpr StageNum finish_ack = 4;

        static constexpr StageNum finish = finish_ack;

    };

    struct N_Broadcast {
        static constexpr StageNum get_rule = C_Broadcast::send_rule;

        static constexpr StageNum send_rule = C_Broadcast::send_rule;

        static constexpr StageNum get_data = C_Broadcast::send_data;

        static constexpr StageNum send_data = C_Broadcast::send_data;

        static constexpr StageNum finish_ack = C_Broadcast::finish_ack;

        static constexpr StageNum finish = C_Broadcast::finish_ack;

    };

}