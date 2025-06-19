//
// Created by taganyer on 25-5-24.
//
#pragma once


#include <tdcf/base/Types.hpp>
#include <tdcf/base/Version.hpp>

namespace tdcf {

    enum class OperationType : uint8_t;

    class MetaData {
    public:
        constexpr MetaData() = default;

        constexpr MetaData(const MetaData&) = default;

        explicit constexpr MetaData(unsigned version) : _v(version) {};

        explicit constexpr MetaData(unsigned version, OperationType type) :
            _v(version), operation_type(type) {};

        explicit constexpr MetaData(Version version) : _v(version) {};

        explicit constexpr MetaData(Version version, OperationType type) :
            _v(version), operation_type(type) {};

        MetaData& operator=(const MetaData&) = default;

        [[nodiscard]] static unsigned serialize_size() {
            return sizeof(MetaData);
        };

        void serialize(void *buffer) const {
            auto ptr = static_cast<unsigned *>(buffer);
            ptr[0] = htonl(_v.version);
            ptr[1] = htonl(
                ((unsigned) operation_type << 24) +
                ((unsigned) data_type << 16) +
                ((unsigned) progress_type << 8) +
                ((unsigned) stage << 0));
            ptr[2] = htonl(serial);
            ptr[3] = htonl(data4[0]);
        };

        void deserialize(const void *buffer) {
            auto ptr = static_cast<const unsigned *>(buffer);
            _v.version = ntohl(ptr[0]);
            unsigned t = ntohl(ptr[1]);
            constexpr unsigned mask = 0xff;
            operation_type = static_cast<OperationType>((t & (mask << 24)) >> 24);
            data_type = static_cast<SerializableBaseTypes>((t & (mask << 16)) >> 16);
            progress_type = static_cast<ProgressType>((t & (mask << 8)) >> 16);
            stage = static_cast<uint8_t>((t & (mask << 0)) >> 0);
            serial = ntohl(ptr[2]);
            data4[0] = ntohl(ptr[3]);
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
        friend bool operator op(const MetaData& left, const MetaData& right) { \
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
        return v._v.version;
    }
};
