//
// Created by taganyer on 25-5-15.
//
#pragma once

#include <cassert>
#include <queue>
#include <tinyBackend/Base/Detail/NoCopy.hpp>

#include "../frame/Commander.hpp"
#include "../frame/Data.hpp"
#include "../frame/Identity.hpp"
#include "../frame/Processor.hpp"

namespace tdcf {

    class Cluster : Base::NoCopy {
    public:
        Cluster(IdentityPtr ip, CommanderPtr cp, ProcessorPtr pp) :
            _id(std::move(ip)),
            _commander(std::move(cp)),
            _processor(std::move(pp)) {
            assert(_id);
            assert(cp);
            assert(pp);
        };

        virtual ~Cluster() = default;

        virtual void join_in_cluster(const IdentityPtr& cluster_id) = 0;

        virtual StatusFlag set_root_node(const IdentityPtr& id) = 0;

        virtual void broadcast(MetaDataPtr meta_data_ptr,  ProcessingRulesPtr rule_ptr) = 0;

        virtual void scatter(MetaDataPtr meta_data_ptr,  ProcessingRulesPtr rule_ptr) = 0;

        virtual void reduce(MetaDataPtr meta_data_ptr,  ProcessingRulesPtr rule_ptr) = 0;

        virtual void all_gather(MetaDataPtr meta_data_ptr,  ProcessingRulesPtr rule_ptr) = 0;

        virtual void all_reduce(MetaDataPtr meta_data_ptr,  ProcessingRulesPtr rule_ptr) = 0;

        virtual void reduce_scatter(MetaDataPtr meta_data_ptr,  ProcessingRulesPtr rule_ptr) = 0;

        virtual void all_to_all(MetaDataPtr meta_data_ptr,  ProcessingRulesPtr rule_ptr) = 0;

        virtual StatusFlag handle_a_loop() = 0;

    protected:
        IdentityPtr _id, _parent;

        CommanderPtr _commander;

        ProcessorPtr _processor;

    };

    namespace detail {

        using ClusterCommandType = int;

        struct ClusterCommand {
            ClusterCommandType type;

            explicit ClusterCommand(ClusterCommandType t) : type(t) {};

            virtual ~ClusterCommand() = default;
        };

        using ClusterCommandPtr = std::unique_ptr<ClusterCommand>;

        using ClusterCommandQueue = std::queue<ClusterCommandPtr>;

    }

}
