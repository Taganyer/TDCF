//
// Created by taganyer on 25-5-21.
//
#pragma once

#include <memory>

namespace tdcf {

    using SerializableType = int;

    class Serializable;

    using SerializablePtr = std::shared_ptr<Serializable>;

    class Serializable {
    public:
        Serializable() = default;

        virtual ~Serializable() = default;

        virtual unsigned serialize_size() const;

        virtual void serialize(void* buffer) const = 0;

        virtual SerializableType type() const = 0;

        static SerializablePtr deserialize(const void* data, unsigned size);

    };



}
