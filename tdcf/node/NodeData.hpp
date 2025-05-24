//
// Created by taganyer on 25-5-22.
//
#pragma once

#include <memory>
#include <tdcf/frame/Commander.hpp>
#include <tdcf/frame/Identity.hpp>
#include <tdcf/frame/Processor.hpp>
#include <tdcf/frame/Transmitter.hpp>

namespace tdcf {

    class NodeData : public Serializable {
    public:
        NodeData() = default;

        NodeData(Identity* ip, Transmitter* tp, Commander* cp, Processor* pp) :
            _id(ip), _transmitter(tp), _commander(cp), _processor(pp) {};

        ~NodeData() override = default;

        [[nodiscard]] SerializableType base_type() const final {
            return static_cast<SerializableType>(SerializableBaseTypes::NodeData);
        };

        virtual StatusFlag handle_a_loop() = 0;

    protected:
        Identity* _id = nullptr;

        Transmitter* _transmitter = nullptr;

        Commander* _commander = nullptr;

        Processor* _processor = nullptr;

        friend class Node;

    };

    using NodeDataPtr = std::shared_ptr<NodeData>;

    enum class NodeDataTypes {
        StarNode,
    };

}
