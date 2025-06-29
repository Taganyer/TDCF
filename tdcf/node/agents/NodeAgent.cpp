//
// Created by taganyer on 25-5-25.
//

#include <tdcf/base/Errors.hpp>
#include <tdcf/handle/Handle.hpp>
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

StatusFlag NodeAgent::handle_received_message(uint32_t from_id, const MetaData& meta,
                                              SerializablePtr& data, Handle& handle) {
    if (meta.operation_type == OperationType::Close) {
        assert(!data);
        StatusFlag flag = end_agent(meta, handle);
        TDCF_CHECK_SUCCESS(flag);
        return StatusFlag::ClusterOffline;
    }
    if (data->base_type() == (int) SerializableBaseTypes::ProcessingRules) {
        assert(handle.progress_events.find(meta) == handle.progress_events.end());
        assert(handle.root_serial() == from_id);
        auto rule = std::dynamic_pointer_cast<ProcessingRules>(data);
        return create_progress(meta, rule, handle);
    }

    auto iter = handle.progress_events.find(meta);
    if (iter == handle.progress_events.end()) return StatusFlag::Success;

    if (meta.operation_type == OperationType::Error) {
        iter->second->handle_error(handle);
        return StatusFlag::EventEnd;
    }

    Variant variant(data);
    return iter->second->handle_event(meta, variant, handle);
}
