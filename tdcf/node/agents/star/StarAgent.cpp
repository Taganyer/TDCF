//
// Created by taganyer on 25-6-15.
//
#include <tdcf/base/Errors.hpp>
#include <tdcf/detail/NodeInformation.hpp>
#include <tdcf/node/agents/star/StarAgent.hpp>

using namespace tdcf;

/// TODO
StatusFlag StarAgent::init(NodeInformation& info) {
    return StatusFlag::Success;
}

StatusFlag StarAgent::handle_connect_request(NodeInformation& info, IdentityPtr& id) {
}

StatusFlag StarAgent::handle_disconnect_request(NodeInformation& info, IdentityPtr& id) {
}

StatusFlag StarAgent::handle_data(NodeInformation& info, IdentityPtr& id,
                                  const MetaData& meta, SerializablePtr& data) {
}

StatusFlag StarAgent::create_progress(NodeInformation& info, const MetaData& meta, SerializablePtr& data) {
    assert(info.progress_events.find(meta) == info.progress_events.end());
}
