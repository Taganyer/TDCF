//
// Created by taganyer on 25-5-15.
//
#pragma once

#include <cassert>

#include "Commander.hpp"
#include "Identity.hpp"
#include "Processor.hpp"
#include "Transmitter.hpp"

namespace tdcf {

    class NodeFlag {};

    class Node {
    public:
        Node(IdentityPtr ip, TransmitterPtr tp, CommanderPtr cp, ProcessorPtr pp) :
            _id(std::move(ip)),
            _transmitter(std::move(tp)),
            _commander(std::move(cp)),
            _processor(std::move(pp)) {
            assert(_transmitter);
            assert(_commander);
            assert(_processor);
        };

        virtual ~Node() = default;

        virtual NodeFlag join_in_cluster(const Identity& cluster_id) = 0;

        virtual void handle_a_loop() = 0;

    protected:
        IdentityPtr _id;

        TransmitterPtr _transmitter;

        CommanderPtr _commander;

        ProcessorPtr _processor;

    };

}
