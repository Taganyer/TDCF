//
// Created by taganyer on 25-8-3.
//

#include <test/frame1/Communicator1.hpp>
#include <test/frame1/ComponentCreator1.hpp>
#include <test/frame1/Identity1.hpp>
#include <test/frame1/ProcessingRules1.hpp>
#include <test/frame1/Processor1.hpp>

using namespace test;

using namespace tdcf;


CommunicatorPtr ComponentCreator1::getCommunicator(uint32_t id) {
    return std::make_shared<Communicator1>(id, share);
}

IdentityPtr ComponentCreator1::getIdentityPtr(uint32_t id) {
    return std::make_shared<Identity1>(id);
}

ProcessorPtr ComponentCreator1::getProcessPtr(uint32_t id) {
    return std::make_shared<Processor1>(id);
}

ProcessingRulesPtr ComponentCreator1::getProcessingRulesPtr(uint32_t serial,
                                                            OperationType type,
                                                            CallBack call_back) {
    std::shared_ptr<ProcessingRules1> rule = std::make_shared<ProcessingRules1>(++serial, type);
    rule->set_callback(std::move(call_back));
    return rule;
}

ComponentCreatorPtr ComponentCreator1::get() {
    return std::make_shared<ComponentCreator1>();
}
