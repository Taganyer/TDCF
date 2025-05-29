//
// Created by taganyer on 25-5-22.
//
#pragma once

#include <string>
#include <tdcf/base/Errors.hpp>
#include <tdcf/base/Serializable.hpp>

namespace tdcf {

    class Interpreter {
    public:
        using String = const std::string&;

        Interpreter() = default;

        virtual ~Interpreter() = default;

        StatusFlag decode(SerializableType base_type, SerializableType derived_type,
                          String data, SerializablePtr& buffer_ptr) {
            auto base = static_cast<SerializableBaseTypes>(base_type);
            switch (base) {
                case SerializableBaseTypes::NodeAgent:
                    return decode_NodeAgent(derived_type, data, buffer_ptr);
                case SerializableBaseTypes::MetaData:
                    return decode_MetaData(derived_type, data, buffer_ptr);
                case SerializableBaseTypes::ProcessingRules:
                    return decode_ProcessRules(derived_type, data, buffer_ptr);
                case SerializableBaseTypes::Identity:
                    return decode_Identity(derived_type, data, buffer_ptr);
                case SerializableBaseTypes::Command:
                    return decode_Command(derived_type, data, buffer_ptr);
                default:
                    TDCF_RAISE_ERROR("Unknown SerializableBaseTypes::type")
            }
        };

    protected:
        virtual StatusFlag decode_MetaData(SerializableType derived_type, String data,
                                           SerializablePtr& buffer_ptr) = 0;

        virtual StatusFlag decode_ProcessRules(SerializableType derived_type, String data,
                                               SerializablePtr& buffer_ptr) = 0;

        virtual StatusFlag decode_Identity(SerializableType derived_type, String data,
                                           SerializablePtr& buffer_ptr) = 0;

    private:
        StatusFlag decode_NodeAgent(SerializableType derived_type, String data,
                                   SerializablePtr& buffer_ptr);

        StatusFlag decode_Command(SerializableType derived_type, String data,
                                  SerializablePtr& buffer_ptr);

    };

    using InterpreterPtr = std::shared_ptr<Interpreter>;

}
