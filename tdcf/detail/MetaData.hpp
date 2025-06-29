//
// Created by taganyer on 25-5-24.
//
#pragma once

#ifdef _WIN32
    #include <winsock2.h>
    #pragma comment(lib, "ws2_32.lib")
#else
#include <netinet/in.h>
#endif

#include <tdcf/base/Types.hpp>

namespace tdcf {

    enum class OperationType : uint8_t;

    class MetaData {
    public:
        constexpr MetaData() = default;

        constexpr MetaData(const MetaData&) = default;

        explicit constexpr MetaData(uint32_t v) : version(v) {};

        explicit constexpr MetaData(uint32_t v, OperationType type) :
            version(v), operation_type(type) {};

        MetaData& operator=(const MetaData&) = default;

        [[nodiscard]] static uint32_t serialize_size() {
            return sizeof(MetaData);
        };

        void serialize(void *buffer) const {
            auto ptr = static_cast<uint32_t *>(buffer);
            ptr[0] = htonl(version);
            ptr[1] = htonl(
                ((uint32_t) operation_type << 24) +
                ((uint32_t) data_type << 16) +
                ((uint32_t) progress_type << 8) +
                ((uint32_t) stage << 0));
            ptr[2] = htonl(serial);
            ptr[3] = htonl(data4[0]);
        };

        void deserialize(const void *buffer) {
            auto ptr = static_cast<const uint32_t *>(buffer);
            version = ntohl(ptr[0]);
            uint32_t t = ntohl(ptr[1]);
            constexpr uint32_t mask = 0xff;
            operation_type = static_cast<OperationType>((t & (mask << 24)) >> 24);
            data_type = static_cast<SerializableBaseTypes>((t & (mask << 16)) >> 16);
            progress_type = static_cast<ProgressType>((t & (mask << 8)) >> 16);
            stage = static_cast<uint8_t>((t & (mask << 0)) >> 0);
            serial = ntohl(ptr[2]);
            data4[0] = ntohl(ptr[3]);
        };

#define MetaDataOF(op) \
        friend bool operator op(const MetaData& left, const MetaData& right) { \
            return left.version op right.version; \
        };

        MetaDataOF(==)

        MetaDataOF(!=)

        MetaDataOF(<)

        MetaDataOF(>)

        MetaDataOF(<=)

        MetaDataOF(>=)

#undef MetaDataOF

        uint32_t version = 0;
        OperationType operation_type = OperationType::Null;
        SerializableBaseTypes data_type = SerializableBaseTypes::Null;
        ProgressType progress_type = ProgressType::Null;

        uint8_t stage = 0;
        uint32_t serial = 0;

        union {
            uint8_t data1[4];
            uint16_t data2[2];
            uint32_t data4[1] {};
        };

    };

}

template <>
struct std::hash<tdcf::MetaData> {
    size_t operator()(const tdcf::MetaData& v) const noexcept {
        return v.version;
    }
};
