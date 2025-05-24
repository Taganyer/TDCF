//
// Created by taganyer on 25-5-15.
//
#pragma once

#include <queue>
#include <utility>
#include <tdcf/node/Node.hpp>

namespace tdcf {

    class Cluster : public Node {
    public:
        Cluster(IdentityPtr ip, TransmitterPtr tp, CommanderPtr cp, ProcessorPtr pp) :
            Node(std::move(ip), std::move(tp), std::move(cp), std::move(pp)) {};

        virtual void broadcast(MetaDataPtr meta_data_ptr, ProcessingRulesPtr rule_ptr);

        virtual void scatter(MetaDataPtr meta_data_ptr, ProcessingRulesPtr rule_ptr);

        virtual void reduce(MetaDataPtr meta_data_ptr, ProcessingRulesPtr rule_ptr);

        virtual void all_gather(MetaDataPtr meta_data_ptr, ProcessingRulesPtr rule_ptr);

        virtual void all_reduce(MetaDataPtr meta_data_ptr, ProcessingRulesPtr rule_ptr);

        virtual void reduce_scatter(MetaDataPtr meta_data_ptr, ProcessingRulesPtr rule_ptr);

        virtual void all_to_all(MetaDataPtr meta_data_ptr, ProcessingRulesPtr rule_ptr);

    protected:
        using ClusterCommandType = int;

        struct ClusterCommand {
            ClusterCommandType type;

            explicit ClusterCommand(ClusterCommandType t) : type(t) {};

            virtual ~ClusterCommand() = default;
        };

        using ClusterCommandPtr = std::unique_ptr<ClusterCommand>;

        using ClusterCommandQueue = std::queue<ClusterCommandPtr>;

        ClusterCommandQueue _command_queue;

        struct Broadcast;

        struct Scatter;

        struct Reduce;

        struct AllGather;

        struct AllReduce;

        struct ReduceScatter;

        struct AllToAll;

        enum {
            _Broadcast,
            _Scatter,
            _Reduce,
            _AllGather,
            _AllReduce,
            _ReduceScatter,
            _AllToAll,
        };

    };

}
