//
// Created by taganyer on 25-6-12.
//
#pragma once

#include <tdcf/detail/Serializable.hpp>

namespace tdcf {

    class ProcessingRules : public Serializable {
    public:
        ProcessingRules() = default;

        [[nodiscard]] SerializableType base_type() const final {
            return static_cast<SerializableType>(SerializableBaseType::ProcessingRules);
        };

        /// 由 root 节点调用，
        virtual void finish_callback() = 0;

    };

    using ProcessingRulesPtr = std::shared_ptr<ProcessingRules>;

}
