//
// Created by taganyer on 25-5-24.
//
#pragma once

#include <tdcf/detail/Serializable.hpp>
#include <tdcf/detail/MetaData.hpp>

namespace tdcf {

    class Message : public Serializable {
    public:
        Message() = default;

        explicit Message(const MetaData &meta) : meta_data(meta) {};

        [[nodiscard]] uint32_t serialize_size() const final {
            return MetaData::serialize_size();
        };

        bool serialize(void *buffer, uint32_t buffer_size) const final {
            if (buffer_size < MetaData::serialize_size()) return false;
            meta_data.serialize(buffer);
            return true;
        };

        bool deserialize(const void *buffer, uint32_t buffer_size) final {
            if (buffer_size < MetaData::serialize_size()) return false;
            meta_data.deserialize(buffer);
            return true;
        };

        [[nodiscard]] SerializableType base_type() const final {
            return static_cast<SerializableType>(SerializableBaseTypes::Message);
        };

        [[nodiscard]] SerializableType derived_type() const final {
            return 0;
        };

        MetaData meta_data;

    };

}
