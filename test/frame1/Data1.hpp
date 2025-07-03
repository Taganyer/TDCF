//
// Created by taganyer on 25-7-3.
//
#pragma once

#include <cstring>
#include <tdcf/frame/Data.hpp>
#include <vector>

namespace test {

    class Data1 : public tdcf::Data {
    public:
        using _Data = std::pair<uint32_t, uint32_t>;

        Data1() = default;

        explicit Data1(uint32_t src) : src(src) {};

        [[nodiscard]] uint32_t serialize_size() const override {
            return data.size() * sizeof(_Data) + sizeof(uint32_t);
        };

        bool serialize(void *buffer, uint32_t buffer_size) const override {
            if (buffer_size < serialize_size()) return false;
            auto ptr = static_cast<uint32_t *>(buffer);
            *ptr = data.size();
            std::memcpy(ptr + 1, data.data(), sizeof(_Data) * data.size());
            return true;
        };

        bool deserialize(const void *buffer, uint32_t buffer_size) override {
            if (buffer_size < 4) return false;
            auto ptr = static_cast<const uint32_t *>(buffer);
            data.resize(*ptr);
            std::memcpy(data.data(), ptr + 1, sizeof(_Data) * *ptr);
            return true;
        };

        [[nodiscard]] tdcf::SerializableType derived_type() const override {
            return 1;
        };

        uint32_t src = -1;

        std::vector<_Data> data;

    };

}
