// Copyright (c) 2024 Francesco Cavaliere
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>.

#ifndef CFT_SRC_UTILS_ASSERT_HPP
#define CFT_SRC_UTILS_ASSERT_HPP

// Testing for failed assertions is not trivial with Catch2.
// This header provides a way to throw exceptions on failed assertions in debug mode, which can be
// caught by Catch2. In this way, we can test that out-of-contract input failss as expected (in
// debug mode). To activare this special asserts, define CFT_ASSERT_FAIL_THROWS before including
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
// (This is the price to catch comptime errors with C++11)
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
