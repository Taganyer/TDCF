//
// Created by taganyer on 25-5-22.
//
#pragma once

#include <tdcf/frame/Identity.hpp>

namespace tdcf {
    struct CommunicatorEvent;

    struct NodeInformation;

    struct NodeEventData;

    class NodeAgent : public Serializable {
    public:
        /// 通过此函数反序列化 NodeAgent。
        static StatusFlag deserialize_NodeAgent(const void *buffer, unsigned buffer_size,
                                                SerializableType derived_type, SerializablePtr& buffer_ptr);

        [[nodiscard]] SerializableType base_type() const final {
            return static_cast<SerializableType>(SerializableBaseTypes::NodeAgent);
        };

        virtual StatusFlag init(NodeInformation& info, NodeEventData& data) = 0;

        virtual StatusFlag analysis_message(NodeEventData& data, CommunicatorEvent& event) = 0;

        virtual StatusFlag handle_received_message(NodeInformation& info, NodeEventData& data,
                                                   IdentityPtr& id, SerializablePtr& message) = 0;

        virtual StatusFlag handle_connect_request(NodeInformation& info, NodeEventData& data,
                                                  IdentityPtr& id) = 0;

        virtual StatusFlag handle_disconnect_request(NodeInformation& info, NodeEventData& data,
                                                     IdentityPtr& id) = 0;

        virtual StatusFlag handle_event(NodeInformation& info, NodeEventData& data) = 0;

    };

    using NodeAgentPtr = std::shared_ptr<NodeAgent>;

    enum class NodeDataTypes {
        StarNode,
    };

}
