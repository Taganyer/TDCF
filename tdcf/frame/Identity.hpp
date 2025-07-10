//
// Created by taganyer on 25-5-15.
//
#pragma once

#include <tdcf/detail/Serializable.hpp>

namespace tdcf {

    class Identity : public Serializable {
    public:
        using Uid = uint32_t;

        [[nodiscard]] virtual Uid guid() const = 0;

        [[nodiscard]] bool equal_to(const Identity& other) const {
            return guid() == other.guid();
        };

        [[nodiscard]] bool less_than(const Identity& other) const {
            return guid() < other.guid();
        };

        [[nodiscard]] SerializableType base_type() const final {
            return static_cast<SerializableType>(SerializableBaseType::Identity);
        };

    };

    using IdentityPtr = std::shared_ptr<Identity>;

    struct IdentityPtrLess {
        bool operator()(const IdentityPtr& lhs, const IdentityPtr& rhs) const {
            if (lhs && rhs) return lhs->less_than(*rhs);
            return !lhs;
        };
    };

    struct IdentityPtrEqual {
        bool operator()(const IdentityPtr& lhs, const IdentityPtr& rhs) const {
            if (lhs && rhs) return lhs->equal_to(*rhs);
            return !lhs && !rhs;
        };
    };
}
