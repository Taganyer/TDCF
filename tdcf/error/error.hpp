//
// Created by taganyer on 25-5-23.
//
#pragma once

#include <cassert>
#include <sstream>
#include <stdexcept>
#include <tinyBackend/Base/Detail/config.hpp>

#include "../frame/StatusFlag.hpp"

#if __GNUC__ || __clang__

#define TDCF_FUN_NAME __PRETTY_FUNCTION__

#elif _MSC_VER

#define TDCF_FUN_NAME __FUNCSIG__

#endif

#define TDCF_CHECK_EXPR(expr) \
if (unlikely(!(expr))) { \
    std::stringstream ss; \
    ss << "tdcf error in " << TDCF_FUN_NAME << " { " << #expr << " }\n"; \
    throw std::runtime_error(ss.str()); \
}

#define TDCF_CHECK_TYPE(expr, expect_type) \
if (StatusFlag status = (expr); unlikely(status == expect_type)) { \
    std::stringstream ss; \
    ss << "tdcf error in " << TDCF_FUN_NAME ; \
    ss << " { " << #expr << " }\n"; \
    ss << "\texpect type is " << status_flag_name(expect_type); \
    ss << " real is " << status_flag_name(status); \
    throw std::runtime_error(ss.str()); \
}

#define TDCF_CHECK_SUCCESS(expr) TDCF_CHECK_TYPE(expr, StatusFlag::Success)
