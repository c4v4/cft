// SPDX-FileCopyrightText: 2024 Francesco Cavaliere <francescocava95@gmail.com>
// SPDX-License-Identifier: MIT

#ifndef CFT_SRC_UTILS_ASSERT_HPP
#define CFT_SRC_UTILS_ASSERT_HPP

// Testing for failed assertions is not trivial with Catch2/doctest.
// This header provides a way to throw exceptions on failed assertions in debug mode, which can be
// caught by Catch2/doctest. In this way, we can test that out-of-contract input fails as expected
// (in debug mode). To activare this special asserts, define CFT_ASSERT_FAIL_THROWS before including
// this header or at compile time.

#ifdef CFT_ASSERT_FAIL_THROWS  // Activates throwing exceptions on failed assertions
#undef assert

#ifndef NDEBUG

#include <stdexcept>  // IWYU pragma: export

// NOTE: In case of constexpr function, if an assertion fails, the error will say something like
// "expression ‘<throw-expression>’ is not a constant expression", which is not very informative.
// What that really means is "an assertion failed, don't look here, look at the assertion and fix
// the issue.".
// The assert should be at the location corresponding to the compiler error: "note: in expansion of
// macro ‘assert’"
#define assert(expr)                                                                               \
    (static_cast<void>((expr) ? 0                                                                  \
                              : throw std::runtime_error(__FILE__ ":" + std::to_string(__LINE__) + \
                                                         " Assertion failed: " #expr)))

#else

#define assert(expr) (static_cast<void>(0))

#endif  // NDEBUG

#else

// Fallaback to the standard assert
#include <cassert>  // IWYU pragma: export

#endif  // CFT_ASSERT_FAIL_THROWS

#endif /* CFT_SRC_UTILS_ASSERT_HPP */
