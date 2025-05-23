//
// Created by taganyer on 25-5-15.
//
#pragma once

#include <cassert>
#include <tinyBackend/Base/Detail/NoCopy.hpp>

#include "NodeData.hpp"
#include "../frame/Commander.hpp"
#include "../frame/Identity.hpp"
#include "../frame/Processor.hpp"
#include "../frame/Transmitter.hpp"

namespace tdcf {

    class Node : Base::NoCopy {
    public:
        Node(IdentityPtr ip, TransmitterPtr tp, CommanderPtr cp, ProcessorPtr pp) :
            _id(std::move(ip)),
            _transmitter(std::move(tp)),
            _commander(std::move(cp)),
            _processor(std::move(pp)) {
            assert(_id);
            assert(_transmitter);
            assert(_commander);
            assert(_processor);
        };

        ~Node() = default;

        [[nodiscard]] StatusFlag join_in_cluster(const IdentityPtr& cluster_id);

        [[nodiscard]] StatusFlag handle_a_loop();

    private:
        IdentityPtr _id, _cluster_id;

        TransmitterPtr _transmitter;

        CommanderPtr _commander;

        ProcessorPtr _processor;

        NodeDataPtr _node_data;

    };

}
