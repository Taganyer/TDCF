//
// Created by taganyer on 25-5-15.
//
#pragma once

#include <tdcf/detail/Serializable.hpp>

namespace tdcf {

    class Identity : public Serializable {
    public:
        static constexpr SerializableType BaseType = 1;

        virtual bool operator==(const Identity& other) const = 0;

        virtual bool operator<(const Identity& other) const = 0;

        [[nodiscard]] SerializableType base_type() const final {
            return static_cast<SerializableType>(SerializableBaseTypes::Identity);
        };

    };

    using IdentityPtr = std::shared_ptr<Identity>;

}


template<>
struct std::less<tdcf::IdentityPtr> {
    size_t operator()(const tdcf::IdentityPtr& lhs, const tdcf::IdentityPtr& rhs) const {
        return *lhs < *rhs;
    };
};

template<>
struct std::equal_to<tdcf::IdentityPtr> {
    size_t operator()(const tdcf::IdentityPtr& lhs, const tdcf::IdentityPtr& rhs) const {
        return *lhs == *rhs;
    };
};
