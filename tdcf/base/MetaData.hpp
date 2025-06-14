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

#include <tdcf/base/Marcos.hpp>

namespace tdcf {

    enum class MetaDataTypes : uint8_t {
        Null,
        Broadcast,
        Scatter,
        Reduce,
        AllGather,
        AllReduce,
        ReduceScatter,
        AllToAll,
        Supplement,
    };

    struct Version {
        constexpr Version() = default;

        constexpr Version(const Version&) = default;

        explicit constexpr Version(unsigned version) : version(version) {};

        Version& operator++() {
            if (likely(version != UINT32_MAX)) ++version;
            else version = 0;
            return *this;
        };

        Version operator++(int) {
            Version temp = *this;
            if (likely(version != UINT32_MAX)) ++version;
            else version = 0;
            return temp;
        };

        Version& operator+=(unsigned _step) {
            unsigned res = UINT32_MAX - version;
            if (likely(_step < res)) version += _step;
            else version = _step - res - 1;
            return *this;
        };

        friend bool operator==(const Version& left, const Version& right) {
            return left.version == right.version;
        };

        friend bool operator!=(const Version& left, const Version& right) {
            return left.version != right.version;
        };

        friend bool operator<(const Version& left, const Version& right) {
            if (left.version < right.version)
                return right.version - left.version <= UINT32_MAX / 2;
            return left.version - right.version > UINT32_MAX / 2;
        };

        friend bool operator>(const Version& left, const Version& right) {
            if (left.version > right.version)
                return left.version - right.version <= UINT32_MAX / 2;
            return right.version - left.version > UINT32_MAX / 2;
        };

        friend bool operator<=(const Version& left, const Version& right) {
            return left == right || left < right;
        };

        friend bool operator>=(const Version& left, const Version& right) {
            return left == right || left > right;
        };

        unsigned version = 0;
    };

    class MetaData {
    public:
        constexpr MetaData() = default;

        constexpr MetaData(const MetaData&) = default;

        explicit constexpr MetaData(unsigned version) : _v(version) {};

        explicit constexpr MetaData(unsigned version, MetaDataTypes type) :
            _v(version), type(type) {};

        explicit constexpr MetaData(Version version) : _v(version) {};

        explicit constexpr MetaData(Version version, MetaDataTypes type) :
            _v(version), type(type) {};

        MetaData& operator=(const MetaData&) = default;

        [[nodiscard]] static unsigned serialize_size() {
            return sizeof(Version) + sizeof(MetaDataTypes) + sizeof(uint8_t) + sizeof(uint8_t)
                + sizeof(unsigned) + sizeof(unsigned);
        };

        void serialize(void *buffer) const {
            auto ptr = static_cast<unsigned *>(buffer);
            ptr[0] = htonl(_v.version);
            ptr[1] = htonl(((unsigned)type << 16) + ((unsigned)stage << 8) + ((unsigned)ack_stage << 0));
            ptr[2] = htonl(serial);
            ptr[3] = htonl(ack_serial);
        };

        void deserialize(const void *buffer) {
            auto ptr = static_cast<const unsigned *>(buffer);
            _v.version = ntohl(ptr[0]);
            unsigned t = ntohl(ptr[1]);
            constexpr unsigned mask = 0xff;
            type = static_cast<MetaDataTypes>((t & (mask << 16)) >> 24);
            stage = static_cast<uint8_t>((t & (mask << 8)) >> 8);
            ack_stage = static_cast<uint8_t>((t & (mask << 0)) >> 0);
            serial = ntohl(ptr[2]);
            ack_serial = ntohl(ptr[3]);
        };

        [[nodiscard]] unsigned data() const { return _v.version; };

        MetaData& operator++() {
            ++_v;
            return *this;
        };

        MetaData operator++(int) {
            MetaData temp = *this;
            ++_v;
            return temp;
        };

        MetaData& operator+=(unsigned _step) {
            _v += _step;
            return *this;
        };

#define MetaDataOF(op) \
        friend bool operator##op(const MetaData& left, const MetaData& right) { \
            return left._v op right._v; \
        };

        MetaDataOF(==)

        MetaDataOF(!=)

        MetaDataOF(<)

        MetaDataOF(>)

        MetaDataOF(<=)

        MetaDataOF(>=)

#undef MetaDataOF

    private:
        Version _v;

        friend class std::hash<MetaData>;

    public:
        MetaDataTypes type = MetaDataTypes::Null;
        uint8_t stage = 0;
        uint8_t ack_stage = 0;
        unsigned serial = 0;
        unsigned ack_serial = 0;
    };

}

namespace std {

    struct hash<tdcf::Version> {
        size_t operator()(tdcf::Version v) const noexcept {
            return v.version;
        };
    };

    struct hash<tdcf::MetaData> {
        size_t operator()(tdcf::MetaData v) const noexcept {
            return v._v.version;
        };
    };

}
