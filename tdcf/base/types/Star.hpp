//
// Created by taganyer on 25-7-9.
//
#pragma once

#include <tdcf/base/Types.hpp>

namespace tdcf::star {

    struct C_Broadcast {
        static constexpr StageNum acquire_data = 1;

        static constexpr StageNum send_rule = 2;

        static constexpr StageNum send_data = 3;

        static constexpr StageNum finish_ack = 4;

    };

    struct N_Broadcast {
        static constexpr StageNum get_rule = C_Broadcast::send_rule;

        static constexpr StageNum get_data = C_Broadcast::send_data;

        static constexpr StageNum finish = C_Broadcast::finish_ack;

    };

    struct A_Broadcast {
        static constexpr StageNum finish_ack = C_Broadcast::finish_ack;

    };

    struct C_Scatter {
        static constexpr StageNum acquire_data = 1;

        static constexpr StageNum send_rule = 2;

        static constexpr StageNum scatter_data = 3;

        static constexpr StageNum send_data = 4;

        static constexpr StageNum finish_ack = 5;

    };

    struct N_Scatter {
        static constexpr StageNum get_rule = C_Scatter::send_rule;

        static constexpr StageNum get_data = C_Scatter::send_data;

        static constexpr StageNum finish = C_Scatter::finish_ack;

    };

    struct A_Scatter {
        static constexpr StageNum send_rule = C_Scatter::send_rule;

        static constexpr StageNum scatter_data = C_Scatter::scatter_data;

        static constexpr StageNum finish_ack = C_Scatter::finish_ack;

    };

    struct C_Reduce {
        static constexpr StageNum acquire_data = 1;

        static constexpr StageNum send_rule = 2;

        static constexpr StageNum reduce_data = 3;

    };

    struct N_Reduce {
        static constexpr StageNum get_rule = C_Reduce::send_rule;

        static constexpr StageNum send_data = C_Reduce::acquire_data;

    };

    struct A_Reduce {
        static constexpr StageNum send_rule = C_Reduce::send_rule;

        static constexpr StageNum acquire_data = C_Reduce::acquire_data;

        static constexpr StageNum reduce_data = C_Reduce::reduce_data;

    };

    struct C_AllReduce {
        static constexpr StageNum acquire_data = 1;

        static constexpr StageNum send_rule = 2;

        static constexpr StageNum reduce_data = 3;

        static constexpr StageNum send_data = 4;

        static constexpr StageNum finish_ack = 5;

    };

    struct N_AllReduce {
        static constexpr StageNum get_rule = C_AllReduce::send_rule;

        static constexpr StageNum send_data1 = C_AllReduce::acquire_data;

        static constexpr StageNum acquire_data2 = C_AllReduce::send_data;

        static constexpr StageNum finish = C_AllReduce::finish_ack;

    };

    struct A_AllReduce {
        static constexpr StageNum acquire_data1 = C_AllReduce::acquire_data;

        static constexpr StageNum send_rule = C_AllReduce::send_rule;

        static constexpr StageNum reduce_data = C_AllReduce::reduce_data;

        static constexpr StageNum finish_ack = C_AllReduce::finish_ack;

    };

    struct C_ReduceScatter {
        static constexpr StageNum acquire_data = 1;

        static constexpr StageNum send_rule = 2;

        static constexpr StageNum reduce_data = 3;

        static constexpr StageNum scatter_data = 4;

        static constexpr StageNum send_data = 5;

        static constexpr StageNum finish_ack = 6;

    };

    struct N_ReduceScatter {
        static constexpr StageNum get_rule = C_ReduceScatter::send_rule;

        static constexpr StageNum send_data1 = C_ReduceScatter::acquire_data;

        static constexpr StageNum acquire_data2 = C_ReduceScatter::send_data;

        static constexpr StageNum finish = C_ReduceScatter::finish_ack;

    };

    struct A_ReduceScatter {
        static constexpr StageNum acquire_data1 = C_ReduceScatter::acquire_data;

        static constexpr StageNum send_rule = C_ReduceScatter::send_rule;

        static constexpr StageNum reduce_data = C_ReduceScatter::reduce_data;

        static constexpr StageNum scatter_data = C_ReduceScatter::scatter_data;

        static constexpr StageNum finish_ack = C_ReduceScatter::finish_ack;

    };

}
