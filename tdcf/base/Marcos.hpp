//
// Created by taganyer on 25-5-24.
//
#pragma once

#ifdef __GNUC__

#define likely(x)      __builtin_expect(!!(x), 1)
#define unlikely(x)    __builtin_expect(!!(x), 0)

#else

#define likely(x)      (x)
#define unlikely(x)    (x)

#endif

#if __GNUC__ || __clang__

#define TDCF_FUN_NAME __PRETTY_FUNCTION__

#elif _MSC_VER

#define TDCF_FUN_NAME __FUNCSIG__

#endif