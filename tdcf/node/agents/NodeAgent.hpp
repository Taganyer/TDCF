//
// Created by taganyer on 25-5-22.
//
#pragma once

#include <tdcf/detail/MetaData.hpp>
#include <tdcf/frame/Identity.hpp>

namespace tdcf {

    struct CommunicatorEvent;

    class NodeInformation;

    class NodeAgent : public Serializable {
    public:
        /// INFO: 用户通过此函数反序列化 NodeAgent。
        static StatusFlag deserialize_NodeAgent(const MetaData& meta, SerializablePtr& buffer_ptr,
                                                const void *buffer, unsigned buffer_size);

        [[nodiscard]] SerializableType base_type() const final {
            return static_cast<SerializableType>(SerializableBaseTypes::NodeAgent);
        };

        /// 不得向 root 发送回应信息。
        virtual StatusFlag init(const MetaData& meta, NodeInformation& info) = 0;

        StatusFlag handle_received_message(IdentityPtr& id, const MetaData& meta,
                                           SerializablePtr& data, NodeInformation& info);

        virtual StatusFlag handle_disconnect_request(IdentityPtr& id, NodeInformation& info) = 0;

    protected:
        virtual StatusFlag create_progress(const MetaData& meta, ProcessingRulesPtr& rule,
                                           NodeInformation& info) = 0;

    };

    using NodeAgentPtr = std::shared_ptr<NodeAgent>;

}
