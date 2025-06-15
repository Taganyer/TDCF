//
// Created by taganyer on 25-6-15.
//
#pragma once

#include <tdcf/detail/EventProgress.hpp>
#include <tdcf/node/agents/NodeAgent.hpp>

namespace tdcf {

    class StarAgent : public NodeAgent {

        StatusFlag init(NodeInformation& node_info) override;

        StatusFlag analysis_message(NodeInformation& info, CommunicatorEvent& event) override;

        StatusFlag handle_received_message(NodeInformation& info, IdentityPtr& id, const MetaData& meta,
                                           SerializablePtr& data) override;

        StatusFlag handle_connect_request(NodeInformation& info, IdentityPtr& id) override;

        StatusFlag handle_disconnect_request(NodeInformation& info, IdentityPtr& id) override;

        StatusFlag handle_event(NodeInformation& info) override;

        class Broadcast : public EventProgress {
        public:
            static StatusFlag create(ProcessingRulesPtr rp, NodeInformation& info);

            StatusFlag handle_event(const MetaData& meta, Variant& data, NodeInformation& node_info) override;

        private:
            explicit Broadcast(ProcessingRulesPtr rp);

            EventProgressAgent* _agent;

        };
    };

}
