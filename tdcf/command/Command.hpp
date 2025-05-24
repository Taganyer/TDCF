//
// Created by taganyer on 25-5-24.
//
#pragma once

#include <../base/Serializable.hpp>

namespace tdcf {

    class Command : public Serializable {
    public:
        [[nodiscard]] SerializableType base_type() const final {
            return static_cast<SerializableType>(SerializableBaseTypes::Command);
        };
    };

    enum class CommandTypes {
        CommandMark,
    };

}