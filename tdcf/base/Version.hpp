//
// Created by taganyer on 25-5-24.
//
#pragma once

#include <cstdint>
#include <functional>
#include <tdcf/base/Marcos.hpp>

namespace tdcf {

    struct Version {
        constexpr Version() = default;

        constexpr Version(const Version&) = default;

        explicit constexpr Version(unsigned version) : version(version) {};

        Version& operator++() {
            if (likely(version != UINT32_MAX)) ++version;
            else version = 0;
            return *this;
        };

        Version operator++(int) {
            Version temp = *this;
            if (likely(version != UINT32_MAX)) ++version;
            else version = 0;
            return temp;
        };

        Version& operator+=(unsigned _step) {
            unsigned res = UINT32_MAX - version;
            if (likely(_step < res)) version += _step;
            else version = _step - res - 1;
            return *this;
        };

        friend bool operator==(const Version& left, const Version& right) {
            return left.version == right.version;
        };

        friend bool operator!=(const Version& left, const Version& right) {
            return left.version != right.version;
        };

        friend bool operator<(const Version& left, const Version& right) {
            if (left.version < right.version)
                return right.version - left.version <= UINT32_MAX / 2;
            return left.version - right.version > UINT32_MAX / 2;
        };

        friend bool operator>(const Version& left, const Version& right) {
            if (left.version > right.version)
                return left.version - right.version <= UINT32_MAX / 2;
            return right.version - left.version > UINT32_MAX / 2;
        };

        friend bool operator<=(const Version& left, const Version& right) {
            return left == right || left < right;
        };

        friend bool operator>=(const Version& left, const Version& right) {
            return left == right || left > right;
        };

        unsigned version = 0;

    };

}

template<>
struct std::hash<tdcf::Version> {
    size_t operator()(tdcf::Version v) const noexcept {
        return v.version;
    };
};
