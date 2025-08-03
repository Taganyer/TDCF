//
// Created by taganyer on 25-7-3.
//
#pragma once

#include <tdcf/frame/Data.hpp>

namespace test {

    class Data1 : public tdcf::Data {
    public:
        Data1() = default;

        explicit Data1(uint32_t src) : src(src), data(src) {};

        [[nodiscard]] uint32_t serialize_size() const override {
            return sizeof(uint32_t) + sizeof(uint32_t);
        };

        bool serialize(void *buffer, uint32_t buffer_size) const override {
            if (buffer_size < serialize_size()) return false;
            auto ptr = static_cast<uint32_t *>(buffer);
            ptr[0] = src;
            ptr[1] = data;
            return true;
        };

        bool deserialize(const void *buffer, uint32_t buffer_size) override {
            if (buffer_size < serialize_size()) return false;
            auto ptr = static_cast<const uint32_t *>(buffer);
            src = ptr[0];
            data = ptr[1];
            return true;
        };

        [[nodiscard]] tdcf::SerializableType derived_type() const override {
            return 1;
        };

        uint32_t src = -1;

        uint32_t data = -1;

    };

}
