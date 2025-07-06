//
// Created by taganyer on 25-5-15.
//
#pragma once

#include <tdcf/detail/Serializable.hpp>

namespace tdcf {

    class Identity : public Serializable {
    public:
        [[nodiscard]] virtual bool equal_to(const Identity& other) const = 0;

        [[nodiscard]] virtual bool less_than(const Identity& other) const = 0;

        [[nodiscard]] SerializableType base_type() const final {
            return static_cast<SerializableType>(SerializableBaseType::Identity);
        };

    };

    using IdentityPtr = std::shared_ptr<Identity>;

}


template<>
struct std::less<tdcf::IdentityPtr> {
    bool operator()(const tdcf::IdentityPtr& lhs, const tdcf::IdentityPtr& rhs) const {
        if (lhs && rhs) return lhs->less_than(*rhs);
        return !lhs;
    };
};

template<>
struct std::equal_to<tdcf::IdentityPtr> {
    bool operator()(const tdcf::IdentityPtr& lhs, const tdcf::IdentityPtr& rhs) const {
        if (lhs && rhs) return lhs->equal_to(*rhs);
        return !lhs && !rhs;
    };
};
