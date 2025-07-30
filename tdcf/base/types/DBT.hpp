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

    struct A_Broadcast {
        static constexpr StageNum send_rule = C_Broadcast::send_rule;

        static constexpr StageNum finish_ack = C_Broadcast::finish_ack;

    };

    struct C_Scatter {
        static constexpr StageNum acquire_data = 1;

        static constexpr StageNum send_rule = 2;

        static constexpr StageNum scatter_data = 3;

        static constexpr StageNum send_data = 4;

        static constexpr StageNum finish_ack = 5;

        static constexpr StageNum finish = finish_ack;

    };

    struct N_Scatter {
        static constexpr StageNum get_rule = C_Scatter::send_rule;

        static constexpr StageNum send_rule = C_Scatter::send_rule;

        static constexpr StageNum get_data = C_Scatter::send_data;

        static constexpr StageNum send_data = C_Scatter::send_data;

        static constexpr StageNum finish_ack = C_Scatter::finish_ack;

        static constexpr StageNum finish = C_Scatter::finish_ack;

    };

    struct A_Scatter {
        static constexpr StageNum send_rule = C_Scatter::send_rule;

        static constexpr StageNum scatter_data = C_Scatter::scatter_data;

        static constexpr StageNum finish_ack = C_Scatter::finish_ack;

    };

    struct C_Reduce {
        static constexpr StageNum self_acquire_data = 1;

        static constexpr StageNum acquire_data = 2;

        static constexpr StageNum send_rule = 3;

        static constexpr StageNum reduce_data = 4;

    };

    struct N_Reduce {
        static constexpr StageNum get_rule = C_Reduce::send_rule;

        static constexpr StageNum send_rule = C_Reduce::send_rule;

        static constexpr StageNum acquire_data = C_Reduce::acquire_data;

        static constexpr StageNum reduce_data = C_Reduce::reduce_data;

        static constexpr StageNum send_data = C_Reduce::acquire_data;

    };

    struct A_Reduce {
        static constexpr StageNum self_acquire_data = C_Reduce::self_acquire_data;

        static constexpr StageNum send_rule = C_Reduce::send_rule;

        static constexpr StageNum acquire_data = C_Reduce::acquire_data;

        static constexpr StageNum reduce_data = C_Reduce::reduce_data;

    };

}