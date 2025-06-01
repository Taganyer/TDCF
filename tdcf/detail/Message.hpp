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

        [[nodiscard]] unsigned serialize_size() const final {
            return MetaData::serialize_size() + serialize_data_size();
        };

        StatusFlag serialize(void *buffer, unsigned buffer_size) const final {
            if (buffer_size < MetaData::serialize_size()) return StatusFlag::FurtherWaiting;
            meta_data.serialize(buffer);
            buffer = (char *) buffer + MetaData::serialize_size();
            buffer_size -= MetaData::serialize_size();
            return serialize_message(buffer, buffer_size);
        };

        StatusFlag deserialize(const void *buffer, unsigned buffer_size) final {
            if (buffer_size < MetaData::serialize_size()) return StatusFlag::FurtherWaiting;
            meta_data.deserialize(buffer);
            buffer = (const char *) buffer + MetaData::serialize_size();
            buffer_size -= MetaData::serialize_size();
            return deserialize_message(buffer, buffer_size);
        };

        MetaData meta_data;

    protected:
        [[nodiscard]] virtual unsigned serialize_data_size() const = 0;

        virtual StatusFlag serialize_message(const void *buffer, unsigned buffer_size) const = 0;

        virtual StatusFlag deserialize_message(const void *buffer, unsigned buffer_size) = 0;

    };

}
