//
// Created by taganyer on 25-5-22.
//
#pragma once

#include <tdcf/base/MetaData.hpp>
#include <tdcf/frame/Identity.hpp>

namespace tdcf {

    struct CommunicatorEvent;

    class NodeInformation;

    class NodeAgent : public Serializable {
    public:
        /// 通过此函数反序列化 NodeAgent。
        static StatusFlag deserialize_NodeAgent(const void *buffer, unsigned buffer_size,
                                                SerializableType derived_type, SerializablePtr& buffer_ptr);

        [[nodiscard]] SerializableType base_type() const final {
            return static_cast<SerializableType>(SerializableBaseTypes::NodeAgent);
        };

        virtual StatusFlag init(NodeInformation& info) = 0;

        virtual StatusFlag analysis_message(NodeInformation& info, CommunicatorEvent& event) = 0;

        virtual StatusFlag handle_received_message(NodeInformation& info, IdentityPtr& id,
                                                   const MetaData& meta, SerializablePtr& data) = 0;

        virtual StatusFlag handle_connect_request(NodeInformation& info, IdentityPtr& id) = 0;

        virtual StatusFlag handle_disconnect_request(NodeInformation& info, IdentityPtr& id) = 0;

        virtual StatusFlag handle_event(NodeInformation& info) = 0;

    };

    using NodeAgentPtr = std::shared_ptr<NodeAgent>;

    enum class NodeDataTypes {
        StarNode,
    };

}
