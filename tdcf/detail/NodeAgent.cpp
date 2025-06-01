//
// Created by taganyer on 25-5-25.
//

#include <tdcf/detail/NodeAgent.hpp>

using namespace tdcf;


StatusFlag NodeAgent::deserialize_NodeAgent(const void *buffer, unsigned buffer_size,
                                            SerializableType derived_type, SerializablePtr& buffer_ptr) {
    return StatusFlag::Success;
}
