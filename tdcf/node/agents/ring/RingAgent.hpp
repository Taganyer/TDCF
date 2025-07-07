//
// Created by taganyer on 25-7-6.
//
#pragma once

#include <tdcf/detail/EventProgress.hpp>
#include <tdcf/node/agents/NodeAgent.hpp>
#include <utility>

namespace tdcf {

    class RingAgent : public NodeAgent {
    public:
        void init(const IdentityPtr& from_id, const MetaData& meta,
                  Handle& handle) override;

        [[nodiscard]] SerializableType derived_type() const override;

        struct RingAgentData {
            IdentityPtr send;
            IdentityPtr receive;

            RingAgentData(IdentityPtr send, IdentityPtr receive) :
                send(std::move(send)), receive(std::move(receive)) {};
        };

    private:
        StatusFlag handle_disconnect(const IdentityPtr& id, Handle& handle) override;

        static void connect_handle(Handle::MessageEvent& event, Handle& handle);

        StatusFlag create_progress(uint32_t version, const MetaData& meta,
                                   ProcessingRulesPtr& rule, Handle& handle) override;

    };

} // tdcf
