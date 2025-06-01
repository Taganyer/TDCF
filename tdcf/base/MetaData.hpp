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

    enum class MetaDataTypes : unsigned {
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

    class MetaData {
    public:
        MetaData() = default;

        MetaData(const MetaData&) = default;

        explicit MetaData(unsigned version) : _version(version) {};

        explicit MetaData(unsigned version, MetaDataTypes type) :
            _version(version), type(type) {};

        MetaData& operator=(const MetaData&) = default;

        [[nodiscard]] static unsigned serialize_size() {
            return sizeof(unsigned) * 2;
        };

        void serialize(void *buffer) const {
            auto ptr = static_cast<unsigned *>(buffer);
            ptr[0] = htonl(_version);
            ptr[1] = htonl(static_cast<unsigned>(type));
        };

        void deserialize(const void *buffer) {
            auto ptr = static_cast<const unsigned *>(buffer);
            _version = ntohl(ptr[0]);
            type = static_cast<MetaDataTypes>(ntohl(ptr[1]));
        };

        [[nodiscard]] unsigned data() const { return _version; };

        MetaData& operator++() {
            if (likely(_version != UINT32_MAX)) ++_version;
            else _version = 0;
            return *this;
        };

        MetaData operator++(int) {
            MetaData temp = *this;
            if (likely(_version != UINT32_MAX)) ++_version;
            else _version = 0;
            return temp;
        };

        MetaData& operator+=(unsigned _step) {
            unsigned res = UINT32_MAX - _version;
            if (likely(_step < res)) _version += _step;
            else _version = _step - res - 1;
            return *this;
        };

        friend bool operator==(const MetaData& left, const MetaData& right) {
            return left._version == right._version;
        };

        friend bool operator!=(const MetaData& left, const MetaData& right) {
            return left._version != right._version;
        };

        friend bool operator<(const MetaData& left, const MetaData& right) {
            if (left._version < right._version)
                return right._version - left._version <= UINT32_MAX / 2;
            return left._version - right._version > UINT32_MAX / 2;
        };

        friend bool operator>(const MetaData& left, const MetaData& right) {
            if (left._version > right._version)
                return left._version - right._version <= UINT32_MAX / 2;
            return right._version - left._version > UINT32_MAX / 2;
        };

        friend bool operator<=(const MetaData& left, const MetaData& right) {
            return left == right || left < right;
        };

        friend bool operator>=(const MetaData& left, const MetaData& right) {
            return left == right || left > right;
        };

    private:
        unsigned _version = 0;

    public:
        MetaDataTypes type = MetaDataTypes::Null;

    };

}
