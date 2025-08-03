//
// Created by taganyer on 25-8-3.
//
#pragma once

#include <test/manager/ComponentCreator.hpp>
#include <test/frame1/Communicator1.hpp>

namespace test {

    class ComponentCreator1 : public ComponentCreator {
    public:
        tdcf::CommunicatorPtr getCommunicator(uint32_t id) override;

        tdcf::IdentityPtr getIdentityPtr(uint32_t id) override;

        tdcf::ProcessorPtr getProcessPtr(uint32_t id) override;

        tdcf::ProcessingRulesPtr getProcessingRulesPtr(uint32_t serial,
                                                       tdcf::OperationType type,
                                                       CallBack call_back) override;

        static ComponentCreatorPtr get();

    private:
        CommShare share;

    };

}
