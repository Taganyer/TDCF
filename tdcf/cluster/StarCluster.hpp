//
// Created by taganyer on 25-5-22.
//
#pragma once

#include <list>
#include <map>
#include <tdcf/cluster/Cluster.hpp>
#include <tdcf/detail/CommandMark.hpp>

namespace tdcf {

    class StarCluster : public Cluster {
    public:
        StarCluster(IdentityPtr idp, TransmitterPtr tp, CommanderPtr cp, ProcessorPtr pp,
                    InterpreterPtr inp, unsigned cluster_size);

        ~StarCluster() override;

        StatusFlag handle_a_loop() override;

    private:
        struct DataStore {
            unsigned task_ref = 0;
            bool connected_client = false, connected_transmitter = false;
            CommanderEventMark commander_event_mark;
            TransmitterEventMark transmitter_event_mark;
        };

        using NodeMap = std::map<IdentityPtr, DataStore, IdentityPtrLess>;

        using RunningCommandList = std::list<ClusterEvent>;

        struct TaskData {};

        using RunningMarkList = std::map<CommandMark, TaskData>;

        SerializablePtr create_node_data();

    };

}
