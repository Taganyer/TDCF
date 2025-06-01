//
// Created by taganyer on 25-5-21.
//
#pragma once

#include <memory>
#include <tdcf/detail/StatusFlag.hpp>

namespace tdcf {

    using SerializableType = int;

    enum class SerializableBaseTypes {
        Identity,
        NodeAgent,
        Data,
        ProcessingRules,
    };

    class Serializable {
    public:
        Serializable() = default;

        virtual ~Serializable() = default;

        [[nodiscard]] virtual unsigned serialize_size() const = 0;

        virtual StatusFlag serialize(void *buffer, unsigned buffer_size) const = 0;

        virtual StatusFlag deserialize(const void *buffer, unsigned buffer_size) = 0;

        [[nodiscard]] virtual SerializableType base_type() const = 0;

        [[nodiscard]] virtual SerializableType derived_type() const = 0;

    };

    using SerializablePtr = std::shared_ptr<Serializable>;

}
