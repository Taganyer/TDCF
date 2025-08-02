//
// Created by taganyer on 25-6-19.
//
#pragma once

#include <cstdint>
#include <tdcf/base/Marcos.hpp>

namespace tdcf {

#define OPERATION_TYPE_ITEM(MOD) \
    MOD(Null), \
    MOD(AgentCreate), \
    MOD(Init), \
    MOD(Close), \
    MOD(Broadcast), \
    MOD(Scatter), \
    MOD(Reduce), \
    MOD(AllReduce), \
    MOD(ReduceScatter),

    enum class OperationType : uint8_t {
        OPERATION_TYPE_ITEM(TDCF_ENUM_MOD)
    };

    constexpr const char* operation_type_name(OperationType type) {
        constexpr const char *item_names[] = {
            OPERATION_TYPE_ITEM(TDCF_NAME_MOD)
        };
        return item_names[static_cast<int>(type)];
    };

#undef OPERATION_TYPE_ITEM

#define SERIALIZABLE_BASE_TYPE(MOD) \
    MOD(Null), \
    MOD(Message), \
    MOD(Identity), \
    MOD(Data), \
    MOD(ProcessingRules),

    enum class SerializableBaseType : uint8_t {
        SERIALIZABLE_BASE_TYPE(TDCF_ENUM_MOD)
    };

    constexpr const char* serializable_base_type_name(SerializableBaseType type) {
        constexpr const char *item_names[] = {
            SERIALIZABLE_BASE_TYPE(TDCF_NAME_MOD)
        };
        return item_names[static_cast<int>(type)];
    };

#undef SERIALIZABLE_BASE_TYPE

    enum class LinkMark : uint8_t {
        Null,
        Create,
        Info,
    };

    enum class ProgressType : uint8_t {
        Null,
        Root,
        Node,
        NodeRoot,
    };

    enum class OperationFlag : uint8_t {
        Success,
        FurtherWaiting,
        Error,
    };

    using StageNum = uint8_t;

    struct ClusterType {
        static constexpr StageNum star = 1;

        static constexpr StageNum ring = 2;

        static constexpr StageNum dbt = 3;

    };

    struct Star {
        static constexpr StageNum start = 1;

        static constexpr StageNum close = 2;

    };

    struct Ring {
        static constexpr StageNum start = 1;

        static constexpr StageNum close = 2;

    };

    struct DBT {
        static constexpr StageNum start = 1;

        static constexpr StageNum init_end = 2;

        static constexpr StageNum close = 2;

    };

    struct Public_Broadcast {
        static constexpr StageNum node_store = 100;

        static constexpr StageNum node_finish_ack = 101;

        static constexpr StageNum agent_receive = node_store;

        static constexpr StageNum agent_finish = node_finish_ack;

    };

    struct Public_Scatter {
        static constexpr StageNum node_store = 100;

        static constexpr StageNum node_finish_ack = 101;

        static constexpr StageNum agent_receive = node_store;

        static constexpr StageNum agent_finish = node_finish_ack;

    };

    struct Public_Reduce {
        static constexpr StageNum node_acquire = 100;

        static constexpr StageNum agent_send = node_acquire;

    };

    struct Public_AllReduce {
        static constexpr StageNum node_acquire = 100;

        static constexpr StageNum node_store = 101;

        static constexpr StageNum node_finish_ack = 102;

        static constexpr StageNum agent_send = node_acquire;

        static constexpr StageNum agent_receive = node_store;

        static constexpr StageNum agent_finish = node_finish_ack;

    };

    struct Public_ReduceScatter {
        static constexpr StageNum node_acquire = 100;

        static constexpr StageNum node_store = 101;

        static constexpr StageNum node_finish_ack = 102;

        static constexpr StageNum agent_send = node_acquire;

        static constexpr StageNum agent_receive = node_store;

        static constexpr StageNum agent_finish = node_finish_ack;

    };

}
