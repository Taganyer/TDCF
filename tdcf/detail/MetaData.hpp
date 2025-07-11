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
            ptr[0] = htonl(root_uid);
            ptr[1] = htonl(version);
            ptr[2] = htonl(
                ((uint32_t) operation_type << 24) +
                ((uint32_t) link_mark << 16) +
                ((uint32_t) progress_type << 8) +
                ((uint32_t) stage << 0));
            ptr[3] = htonl(serial);
            ptr[4] = htonl(rest_data);
            ptr[5] = htonl(data4[0]);
        };

        void deserialize(const void *buffer) {
            auto ptr = static_cast<const uint32_t *>(buffer);
            root_uid = ntohl(ptr[0]);
            version = ntohl(ptr[1]);
            uint32_t t = ntohl(ptr[2]);
            constexpr uint32_t mask = 0xff;
            operation_type = static_cast<OperationType>((t & (mask << 24)) >> 24);
            link_mark = static_cast<LinkMark>((t & (mask << 16)) >> 16);
            progress_type = static_cast<ProgressType>((t & (mask << 8)) >> 16);
            stage = static_cast<uint8_t>((t & (mask << 0)) >> 0);
            serial = ntohl(ptr[3]);
            rest_data = ntohl(ptr[4]);
            data4[0] = ntohl(ptr[5]);
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

        uint32_t root_uid = 0;
        uint32_t version = 0;

        OperationType operation_type = OperationType::Null;
        LinkMark link_mark = LinkMark::Null;
        ProgressType progress_type = ProgressType::Null;

        uint8_t stage = 0;
        uint32_t serial = 0;

        uint32_t rest_data = 0;
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
