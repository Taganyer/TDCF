//
// Created by taganyer on 25-5-25.
//

#include <tdcf/base/Errors.hpp>
#include <tdcf/node/agents/NodeAgent.hpp>

using namespace tdcf;


StatusFlag NodeAgent::deserialize_NodeAgent(const void *buffer, unsigned buffer_size,
                                            SerializableType derived_type, SerializablePtr& buffer_ptr) {
    return StatusFlag::Success;
}

StatusFlag NodeAgent::handle_received_message(NodeInformation& info, IdentityPtr& id,
                                              const MetaData& meta, SerializablePtr& data) {
    if (data->base_type() == (int) SerializableBaseTypes::Data) {
        return handle_data(info, id, meta, data);
    }
    if (data->base_type() == (int) SerializableBaseTypes::ProcessingRules) {
        return create_progress(info, meta, data);
    }
    TDCF_RAISE_ERROR(error type);
}
