//
// Created by taganyer on 25-5-15.
//
#pragma once

#include <tdcf/node/Node.hpp>
#include <utility>

namespace tdcf {

    class Cluster : public Node {
    public:
        Cluster(IdentityPtr ip, CommunicatorPtr cp, ProcessorPtr pp, IdentityPtr root_id) :
            Node(std::move(ip), std::move(cp), std::move(pp), std::move(root_id)) {};

        ~Cluster() override { if (_cluster_started) end_cluster(); };

        void start(unsigned cluster_size) final;

        StatusFlag end_cluster();

        virtual StatusFlag broadcast(ProcessingRulesPtr rule_ptr) = 0;

        virtual StatusFlag scatter(ProcessingRulesPtr rule_ptr) = 0;

        virtual StatusFlag reduce(ProcessingRulesPtr rule_ptr) = 0;

        virtual StatusFlag all_gather(ProcessingRulesPtr rule_ptr) = 0;

        virtual StatusFlag all_reduce(ProcessingRulesPtr rule_ptr) = 0;

        virtual StatusFlag reduce_scatter(ProcessingRulesPtr rule_ptr) = 0;

        virtual StatusFlag all_to_all(ProcessingRulesPtr rule_ptr) = 0;

        [[nodiscard]] bool cluster_started() const { return _cluster_started; };

    protected:
        virtual void cluster_accept(unsigned cluster_size) = 0;

        virtual void cluster_start() = 0;

        virtual void cluster_end() = 0;

        virtual StatusFlag handle_received_message(IdentityPtr& id, const MetaData& meta,
                                                   SerializablePtr& data) = 0;

        virtual StatusFlag handle_disconnect_request(IdentityPtr& id) = 0;

    private:
        StatusFlag handle_message(CommunicatorEvent& event) final;

    };

}
