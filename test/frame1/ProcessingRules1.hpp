//
// Created by taganyer on 25-7-3.
//
#pragma once

#include <tdcf/detail/MetaData.hpp>
#include <tdcf/frame/ProcessingRules.hpp>
#include <test/Log.hpp>

namespace test {

    class ProcessingRules1 : public tdcf::ProcessingRules {
    public:
        ProcessingRules1() = default;

        ProcessingRules1(uint32_t id, tdcf::OperationType type) : _id(id), _operation(type) {};

        [[nodiscard]] uint32_t serialize_size() const override {
            return sizeof(uint32_t) + sizeof(tdcf::OperationType);
        };

        bool serialize(void *buffer, uint32_t buffer_size) const override {
            if (buffer_size < serialize_size()) return false;
            auto ptr1 = static_cast<uint32_t *>(buffer);
            *ptr1 = _id;
            auto ptr2 = reinterpret_cast<tdcf::OperationType *>(ptr1 + 1);
            *ptr2 = _operation;
            return true;
        };

        bool deserialize(const void *buffer, uint32_t buffer_size) override {
            if (buffer_size < serialize_size()) return false;
            auto ptr1 = static_cast<const uint32_t *>(buffer);
            _id = *ptr1;
            auto ptr2 = reinterpret_cast<const tdcf::OperationType *>(ptr1 + 1);
            _operation = *ptr2;
            return true;
        };

        [[nodiscard]] tdcf::SerializableType derived_type() const override {
            return 1;
        };

        [[nodiscard]] uint32_t id() const { return _id; };

        [[nodiscard]] tdcf::OperationType operation() const { return _operation; };

        void finish_callback() override {
            T_INFO << _id << ": " << __FUNCTION__ << " operation: "
                    << tdcf::operation_type_name(_operation);
        };

    private:
        uint32_t _id = -1;

        tdcf::OperationType _operation = tdcf::OperationType::Null;

    };

}
