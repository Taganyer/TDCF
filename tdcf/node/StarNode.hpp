//
// Created by taganyer on 25-5-24.
//
#pragma once

#include <tdcf/node/Node.hpp>

namespace tdcf {

    class StarNode : public Node {
    public:
        StatusFlag handle_broadcast();

        StatusFlag handle_scatter();

        StatusFlag handle_reduce();

        StatusFlag handle_all_gather();

        StatusFlag handle_all_reduce();

        StatusFlag handle_reduce_scatter();

        StatusFlag handle_all_to_all();

        StatusFlag handle_a_loop() override;

    };

}
