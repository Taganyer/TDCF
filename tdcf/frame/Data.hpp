//
// Created by taganyer on 25-5-18.
//
#pragma once

#include <tdcf/base/Serializable.hpp>

namespace tdcf {

    class MetaData : public Serializable {
    public:
        [[nodiscard]] SerializableType base_type() const final {
            return static_cast<SerializableType>(SerializableBaseTypes::MetaData);
        };

    };

    using MetaDataPtr = std::shared_ptr<MetaData>;

    class Data {
    public:
        Data() = default;

        virtual ~Data() = default;

    };

    using DataPtr = std::shared_ptr<Data>;

}
