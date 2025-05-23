//
// Created by taganyer on 25-5-22.
//
#pragma once

#include <memory>

#include "../frame/Commander.hpp"
#include "../frame/Identity.hpp"
#include "../frame/Processor.hpp"
#include "../frame/Transmitter.hpp"

namespace tdcf {

    class NodeData : public Serializable {
    public:
        static constexpr SerializableType BaseType = 4;

        NodeData() = default;

        NodeData(Identity* ip, Transmitter* tp, Commander* cp, Processor* pp) :
            _id(ip), _transmitter(tp), _commander(cp), _processor(pp) {};

        ~NodeData() override = default;

        [[nodiscard]] SerializableType base_type() const final { return BaseType; };

        virtual StatusFlag handle_a_loop() = 0;

    protected:
        Identity* _id = nullptr;

        Transmitter* _transmitter = nullptr;

        Commander* _commander = nullptr;

        Processor* _processor = nullptr;

        friend class Node;

    };

    using NodeDataPtr = std::shared_ptr<NodeData>;

}
