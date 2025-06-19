//
// Created by taganyer on 25-5-25.
//

#include <tdcf/base/Errors.hpp>
#include <tdcf/detail/NodeInformation.hpp>
#include <tdcf/node/agents/NodeAgent.hpp>
#include <tdcf/node/agents/star/StarAgent.hpp>

using namespace tdcf;


StatusFlag NodeAgent::deserialize_NodeAgent(const MetaData& meta, SerializablePtr& buffer_ptr,
                                            const void *buffer, unsigned buffer_size) {
    assert(meta.operation_type == OperationType::AgentCreate);
    if (meta.data1[0] == ClusterType::star) {
        auto ptr = std::make_shared<StarAgent>();
        ptr->deserialize(buffer, buffer_size);
        buffer_ptr = std::move(ptr);
        return StatusFlag::Success;
    }
    TDCF_RAISE_ERROR(error NodeAgent type)
}

StatusFlag NodeAgent::handle_received_message(IdentityPtr& id, const MetaData& meta,
                                              SerializablePtr& data, NodeInformation& info) {
    if (data->base_type() == (int) SerializableBaseTypes::ProcessingRules) {
        assert(info.progress_events.find(meta) == info.progress_events.end());
        assert(info.root_id == id);
        auto rule = std::dynamic_pointer_cast<ProcessingRules>(data);
        return create_progress(meta, rule, info);
    }
    auto iter = info.progress_events.find(meta);
    assert(iter != info.progress_events.end());
    Variant variant(data);
    return iter->second->handle_event(meta, variant, info);
}
