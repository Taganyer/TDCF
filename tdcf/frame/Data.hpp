//
// Created by taganyer on 25-5-18.
//
#pragma once

#include <tdcf/detail/Message.hpp>

namespace tdcf {

    class Data : public Serializable {
    public:
        Data() = default;

        [[nodiscard]] SerializableType base_type() const final {
            return static_cast<SerializableType>(SerializableBaseType::Data);
        };

        [[nodiscard]] SerializableType derived_type() const override {
            return 0;
        };
        //
        // [[nodiscard]] uint32_t serialize_size() const override {
        //     return 0;
        // };
        //
        // bool serialize(void *buffer, uint32_t buffer_size) const override {
        //     return true;
        // };
        //
        // bool deserialize(const void *buffer, uint32_t buffer_size) override {
        //     return true;
        // };

    };

    using DataPtr = std::shared_ptr<Data>;

}
