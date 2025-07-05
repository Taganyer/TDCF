//
// Created by taganyer on 25-5-22.
//
#pragma once

#include <tdcf/detail/MetaData.hpp>
#include <tdcf/detail/EventProgress.hpp>
#include <tdcf/frame/Identity.hpp>
#include <tdcf/frame/ProcessingRules.hpp>

namespace tdcf {

    struct CommunicatorEvent;

    class Handle;

    class NodeAgent : public Serializable {
    public:
        /// INFO: 用户通过此函数反序列化 NodeAgent。
        static StatusFlag deserialize_NodeAgent(const MetaData& meta, SerializablePtr& buffer_ptr,
                                                const void *buffer, unsigned buffer_size);

        [[nodiscard]] SerializableType base_type() const final {
            return static_cast<SerializableType>(SerializableBaseType::NodeAgent);
        };

        /// 不得向 root 发送回应信息。
        virtual StatusFlag init(const MetaData& meta, Handle& handle) = 0;

        StatusFlag handle_received_message(const IdentityPtr& from_id, const MetaData& meta,
                                           Variant& variant, Handle& handle);

    protected:
        virtual StatusFlag create_progress(uint32_t version, const MetaData& meta,
                                           ProcessingRulesPtr& rule, Handle& handle) = 0;

        virtual StatusFlag end_agent(const MetaData& meta, Handle& handle) = 0;

    };

    using NodeAgentPtr = std::shared_ptr<NodeAgent>;

}
