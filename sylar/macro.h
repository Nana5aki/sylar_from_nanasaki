/*
 * @Author: Nana5aki
 * @Date: 2025-01-01 16:40:13
 * @LastEditors: Nana5aki
 * @LastEditTime: 2025-05-18 17:52:57
 * @FilePath: /MySylar/sylar/macro.h
 */
#ifndef __SYLAR_MACRO_H__
#define __SYLAR_MACRO_H__

#include "log.h"
#include "util/util.h"
#include <assert.h>

#if defined __GNUC__ || defined __llvm__
/// LIKCLY 宏的封装, 告诉编译器优化,条件大概率成立
#  define SYLAR_LIKELY(x) __builtin_expect(!!(x), 1)
/// LIKCLY 宏的封装, 告诉编译器优化,条件大概率不成立
#  define SYLAR_UNLIKELY(x) __builtin_expect(!!(x), 0)
#else
#  define SYLAR_LIKELY(x) (x)
#  define SYLAR_UNLIKELY(x) (x)
#endif

/// 断言宏封装
#define SYLAR_ASSERT(x)                                                                  \
  if (SYLAR_UNLIKELY(!(x))) {                                                            \
    SYLAR_LOG_ERROR(SYLAR_LOG_ROOT()) << "ASSERTION: " #x << "\nbacktrace:\n"            \
                                      << sylar::util::BacktraceToString(100, 2, "    "); \
    assert(x);                                                                           \
  }

/// 断言宏封装
#define SYLAR_ASSERT2(x, w)                                                              \
  if (SYLAR_UNLIKELY(!(x))) {                                                            \
    SYLAR_LOG_ERROR(SYLAR_LOG_ROOT()) << "ASSERTION: " #x << "\n"                        \
                                      << w << "\nbacktrace:\n"                           \
                                      << sylar::util::BacktraceToString(100, 2, "    "); \
    assert(x);                                                                           \
  }

#endif