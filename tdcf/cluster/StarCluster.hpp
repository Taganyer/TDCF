//
// Created by taganyer on 25-5-22.
//
#pragma once

#include <set>
#include <tdcf/cluster/Cluster.hpp>

namespace tdcf {

    class StarCluster : public Cluster {
    public:
        StarCluster(IdentityPtr ip, TransmitterPtr tp, CommanderPtr cp, ProcessorPtr pp) :
            Cluster(std::move(ip), std::move(tp), std::move(cp), std::move(pp)) {};

        ~StarCluster() override;

        StatusFlag handle_a_loop() override;

    private:
        std::set<IdentityPtr, IdentityPtrLess> _nodes;

        using Iterator = std::set<IdentityPtr, IdentityPtrLess>::iterator;

        Iterator _root;

    };

}
