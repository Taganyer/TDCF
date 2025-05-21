//
// Created by taganyer on 25-5-15.
//
#pragma once

#include <cassert>

#include "Commander.hpp"
#include "Data.hpp"
#include "Identity.hpp"
#include "Processor.hpp"

namespace tdcf {

    class ClusterFlag {};

    class Cluster {
    public:
        Cluster(IdentityPtr ip, CommanderPtr cp, ProcessorPtr pp) :
            _id(std::move(ip)),
            _commander(std::move(cp)),
            _processor(std::move(pp)) {
            assert(cp);
            assert(pp);
        };

        virtual ~Cluster() = default;

        virtual ClusterFlag join_in_cluster(const Identity& cluster_id) = 0;

        virtual ClusterFlag set_root_node(const Identity& id) = 0;

        virtual ClusterFlag broadcast(const MetaDataPtr& meta_data_ptr) = 0;

        virtual ClusterFlag scatter(const MetaDataPtr& meta_data_ptr) = 0;

        virtual ClusterFlag reduce(const MetaDataPtr& meta_data_ptr) = 0;

        virtual ClusterFlag all_gather(const MetaDataPtr& meta_data_ptr) = 0;

        virtual ClusterFlag all_reduce(const MetaDataPtr& meta_data_ptr) = 0;

        virtual ClusterFlag reduce_scatter(const MetaDataPtr& meta_data_ptr) = 0;

        virtual ClusterFlag all_to_all(const MetaDataPtr& meta_data_ptr) = 0;

        virtual void handle_a_loop() = 0;

    protected:
        IdentityPtr _id;

        CommanderPtr _commander;

        ProcessorPtr _processor;

        virtual ClusterFlag accept(const Identity& id) = 0;

    };

}
