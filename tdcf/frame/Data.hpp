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
            return static_cast<SerializableType>(SerializableBaseTypes::Data);
        };

    };

    using DataPtr = std::shared_ptr<Data>;

}
