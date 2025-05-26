//
// Created by taganyer on 25-5-21.
//
#pragma once

#include <memory>

namespace tdcf {

    using SerializableType = int;

    enum class SerializableBaseTypes {
        NodeAgent,
        MetaData,
        ProcessingRules,
        Identity,
        Command,
    };

    class Serializable {
    public:
        Serializable() = default;

        virtual ~Serializable() = default;

        [[nodiscard]] virtual unsigned serialize_size() const = 0;

        virtual void serialize(void *buffer) const = 0;

        [[nodiscard]] virtual SerializableType base_type() const = 0;

        [[nodiscard]] virtual SerializableType derived_type() const = 0;

    };

    using SerializablePtr = std::shared_ptr<Serializable>;

}
