//
// Created by taganyer on 25-5-21.
//
#pragma once

#include <memory>
#include <tinyBackend/Base/Detail/NoCopy.hpp>

namespace tdcf {

    using SerializableType = int;

    class Serializable : Base::NoCopy {
    public:
        Serializable() = default;

        virtual ~Serializable() = default;

        [[nodiscard]] virtual unsigned serialize_size() const = 0;

        virtual void serialize(void *buffer, unsigned buffer_size, unsigned skip_size) const = 0;

        [[nodiscard]] virtual SerializableType base_type() const = 0;

        [[nodiscard]] virtual SerializableType derived_type() const = 0;

    };

    using SerializablePtr = std::shared_ptr<Serializable>;

}
