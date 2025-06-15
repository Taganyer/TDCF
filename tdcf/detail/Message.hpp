//
// Created by taganyer on 25-5-24.
//
#pragma once

#include <tdcf/detail/Serializable.hpp>
#include <tdcf/base/MetaData.hpp>

namespace tdcf {

    class Message : public Serializable {
    public:
        Message() = default;

        explicit Message(const MetaData &meta) : meta_data(meta) {};

        [[nodiscard]] unsigned serialize_size() const final {
            return MetaData::serialize_size();
        };

        StatusFlag serialize(void *buffer, unsigned buffer_size) const final {
            if (buffer_size < MetaData::serialize_size()) return StatusFlag::FurtherWaiting;
            meta_data.serialize(buffer);
            return StatusFlag::Success;
        };

        StatusFlag deserialize(const void *buffer, unsigned buffer_size) final {
            if (buffer_size < MetaData::serialize_size()) return StatusFlag::FurtherWaiting;
            meta_data.deserialize(buffer);
            return StatusFlag::Success;
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
