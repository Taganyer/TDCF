//
// Created by taganyer on 25-5-15.
//
#pragma once

#include <tdcf/base/Serializable.hpp>

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

    struct IdentityPtrEqual {
        bool operator()(const IdentityPtr& lhs, const IdentityPtr& rhs) const {
            return lhs == rhs;
        };

        bool operator()(const IdentityPtr& lhs, const Identity& rhs) const {
            return *lhs == rhs;
        };
    };

    struct IdentityPtrLess {
        bool operator()(const IdentityPtr& lhs, const IdentityPtr& rhs) const {
            return lhs < rhs;
        };

        bool operator()(const IdentityPtr& lhs, const Identity& rhs) const {
            return *lhs < rhs;
        };
    };

}
