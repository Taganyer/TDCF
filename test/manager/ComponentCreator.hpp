//
// Created by taganyer on 25-8-3.
//
#pragma once

#include <functional>
#include <tdcf/frame/Communicator.hpp>
#include <tdcf/frame/Identity.hpp>
#include <tdcf/frame/ProcessingRules.hpp>
#include <tdcf/frame/Processor.hpp>

namespace test {

    class ComponentCreator {
    public:
        using CallBack = std::function<void()>;

        virtual ~ComponentCreator() = default;

        virtual tdcf::CommunicatorPtr getCommunicator(uint32_t id) = 0;

        virtual tdcf::ProcessorPtr getProcessPtr(uint32_t id) = 0;

        virtual tdcf::IdentityPtr getIdentityPtr(uint32_t id) = 0;

        virtual tdcf::ProcessingRulesPtr getProcessingRulesPtr(uint32_t serial,
                                                               tdcf::OperationType type,
                                                               CallBack call_back) = 0;
    };

    using ComponentCreatorPtr = std::shared_ptr<ComponentCreator>;

}