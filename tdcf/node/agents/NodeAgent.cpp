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

StatusFlag NodeAgent::handle_received_message(const IdentityPtr& from_id, const MetaData& meta,
                                              Variant& variant, Handle& handle) {
    if (meta.operation_type == OperationType::Close) {
        auto& data_ptr = std::get<SerializablePtr>(variant);
        assert(!data_ptr);
        StatusFlag flag = end_agent(meta, handle);
        TDCF_CHECK_SUCCESS(flag);
        return StatusFlag::ClusterOffline;
    }
    if (meta.link_mark == LinkMark::Create) {
        assert(from_id->equal_to(*handle.root_identity()));
        assert(!handle.check_progress(handle.find_progress(meta.version)));

        auto& data_ptr = std::get<SerializablePtr>(variant);
        assert(data_ptr->base_type() == (int) SerializableBaseType::ProcessingRules);
        auto rule = std::dynamic_pointer_cast<ProcessingRules>(data_ptr);
        assert(rule);
        return create_progress(meta.version, meta, rule, handle);
    }

    auto iter = handle.find_progress(meta.version);
    TDCF_CHECK_EXPR(handle.check_progress(iter))
    return iter->second->handle_event(meta, variant, handle);
}
