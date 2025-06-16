//
// Created by taganyer on 25-6-15.
//
#pragma once

#include <tdcf/detail/EventProgress.hpp>
#include <tdcf/node/agents/NodeAgent.hpp>

namespace tdcf {

    class StarAgent : public NodeAgent {
    public:
        StatusFlag init(NodeInformation& info) override;

        StatusFlag handle_connect_request(NodeInformation& info, IdentityPtr& id) override;

        StatusFlag handle_disconnect_request(NodeInformation& info, IdentityPtr& id) override;

    private:
        StatusFlag handle_data(NodeInformation& info, IdentityPtr& id,
                               const MetaData& meta, SerializablePtr& data) override;

        StatusFlag create_progress(NodeInformation& info, const MetaData& meta,
                                   SerializablePtr& data) override;

        class Broadcast : public EventProgress {
        public:
            static StatusFlag create(const MetaData& meta, ProcessingRulesPtr rp, NodeInformation& info);

            StatusFlag handle_event(const MetaData& meta, Variant *data, NodeInformation& node_info) override;

        private:
            explicit Broadcast(ProcessingRulesPtr rp, const MetaData& meta);

            MetaData old_meta;

            EventProgressAgent *_agent = nullptr;

        };
    };

}
