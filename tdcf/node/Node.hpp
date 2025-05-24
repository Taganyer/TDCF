//
// Created by taganyer on 25-5-15.
//
#pragma once

#include <cassert>
#include <tdcf/node/NodeData.hpp>

namespace tdcf {

    class Node : NoCopy {
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

        virtual ~Node() = default;

        [[nodiscard]] virtual StatusFlag join_in_cluster(const IdentityPtr& cluster_id);

        [[nodiscard]] virtual StatusFlag handle_a_loop();

    protected:
        IdentityPtr _id, _cluster_id;

        TransmitterPtr _transmitter;

        CommanderPtr _commander;

        ProcessorPtr _processor;

        NodeDataPtr _node_data;

    };

}
