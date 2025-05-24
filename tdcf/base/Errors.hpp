//
// Created by taganyer on 25-5-23.
//
#pragma once

#include <cassert>
#include <sstream>
#include <stdexcept>
#include <tdcf/base/Marcos.hpp>
#include <tdcf/frame/StatusFlag.hpp>

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
