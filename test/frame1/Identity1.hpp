//
// Created by taganyer on 25-7-3.
//
#pragma once

#include <tdcf/frame/Identity.hpp>

namespace test {

    class Identity1 : public tdcf::Identity {
    public:
        Identity1() = default;

        explicit Identity1(uint32_t id) : _id(id) {};

        [[nodiscard]] uint32_t serialize_size() const override {
            return sizeof(_id);
        };

        bool serialize(void *buffer, uint32_t buffer_size) const override {
            if (buffer_size < serialize_size()) return false;
            *static_cast<uint32_t *>(buffer) = _id;
            return true;
        };

        bool deserialize(const void *buffer, uint32_t buffer_size) override {
            if (buffer_size < serialize_size()) return false;
            _id = *static_cast<const uint32_t *>(buffer);
            return true;
        };

        [[nodiscard]] Uid guid() const override {
            return _id;
        };

    private:
        uint32_t _id = -1;

    };

}
