//
// Created by taganyer on 25-7-3.
//
#pragma once

#include <tdcf/frame/ProcessingRules.hpp>
#include <test/Log.hpp>

namespace test {

    class ProcessingRules1 : public tdcf::ProcessingRules {
    public:
        [[nodiscard]] uint32_t serialize_size() const override {
            return sizeof(uint32_t) + sizeof(uint32_t);
        };

        bool serialize(void *buffer, uint32_t buffer_size) const override {
            if (buffer_size < serialize_size()) return false;
            auto ptr = static_cast<uint32_t *>(buffer);
            ptr[0] = id;
            ptr[1] = operation;
            return true;
        };

        bool deserialize(const void *buffer, uint32_t buffer_size) override {
            if (buffer_size < serialize_size()) return false;
            auto data = static_cast<const uint32_t *>(buffer);
            id = data[0];
            operation = data[1];
            return true;
        };

        [[nodiscard]] tdcf::SerializableType derived_type() const override {
            return 1;
        };

        void finish_callback() override {
            T_INFO << id << ": " << __FUNCTION__ << " operation: " << operation;
        };

        uint32_t id = -1;

        uint32_t operation = 0;

    };

}
