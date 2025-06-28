//
// Created by taganyer on 25-5-21.
//
#pragma once

#include <memory>
#include <tdcf/base/Types.hpp>

namespace tdcf {

    using SerializableType = int;

    enum class SerializableBaseTypes : uint8_t;

    class Serializable {
    public:
        Serializable() = default;

        virtual ~Serializable() = default;

        /// 序列化时，可直接调用；反序列化时，调用 deserialize 成功后才能调用，用于查看对象序列化数据具体大小。
        [[nodiscard]] virtual unsigned serialize_size() const = 0;

        virtual bool serialize(void *buffer, unsigned buffer_size) const = 0;

        virtual bool deserialize(const void *buffer, unsigned buffer_size) = 0;

        [[nodiscard]] virtual SerializableType base_type() const = 0;

        [[nodiscard]] virtual SerializableType derived_type() const = 0;

    };

    using SerializablePtr = std::shared_ptr<Serializable>;

}
