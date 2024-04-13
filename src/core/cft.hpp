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

#ifndef CFT_SRC_CORE_CFT_HPP
#define CFT_SRC_CORE_CFT_HPP


#include <cstdint>
#include <vector>

#include "utils/assert.hpp"  // IWYU pragma:  keep
#include "utils/limits.hpp"
#include "utils/random.hpp"
#include "utils/utility.hpp"

#ifdef NDEBUG
#define CFT_IF_DEBUG(...)
#else
#define CFT_IF_DEBUG(...) __VA_ARGS__
#endif

// Noinline attribute to help profiling specific functions
#if defined(__GNUC__) || defined(__clang__)
#define CFT_NOINLINE __attribute__((noinline))
#elif defined(_MSC_VER)
#define CFT_NOINLINE __declspec(noinline)
#endif

namespace cft {

using cidx_t = int32_t;                    // Type for column indexes
using ridx_t = int16_t;                    // Type for row indexes
using real_t = float;                      // Type for real values
using prng_t = prng_picker<real_t>::type;  // default pseudo-random number generator type

struct CidxAndCost {
    cidx_t idx;
    real_t cost;
};

struct Solution {
    std::vector<cidx_t> idxs = {};
    real_t              cost = limits<real_t>::inf();
};

// Reserved values to mark removed indexes (tombstones)
constexpr cidx_t removed_cidx = limits<cidx_t>::max();
constexpr ridx_t removed_ridx = limits<ridx_t>::max();

// Epsilon value for objective comparisons, for integer costs, 1e-6 for float costs
constexpr real_t epsilon = checked_cast<real_t>(1 - 1e-6);

// Debug checked narrow cast to cidx_t
template <typename T>
constexpr cidx_t as_cidx(T val) {
    return checked_cast<cidx_t>(val);
}

// Debug checked narrow cast to ridx_t
template <typename T>
constexpr ridx_t as_ridx(T val) {
    return checked_cast<ridx_t>(val);
}

// Debug checked narrow cast to ridx_t
template <typename T>
constexpr real_t as_real(T val) {
    return assert(limits<real_t>::min() <= val && val <= limits<real_t>::max()),
           checked_cast<real_t>(val);
}

// User-defined literals for cidx_t with debug and comptime checks)
constexpr cidx_t operator""_C(unsigned long long j) {
    return as_cidx(j);
}

// User-defined literals for ridx_t with debug and comptime checks)
constexpr ridx_t operator""_R(unsigned long long i) {
    return as_ridx(i);
}

// User-defined literals for real_t with debug and comptime checks)
constexpr real_t operator""_F(long double f) {
    return as_real(f);
}

// Since cidx_t could be any integer type, csize provide a (debug) checked way to get the size of a
// container as cidx_t. It also allows to avoid narrowing conversion warnings.
template <typename Cont>
inline cidx_t csize(Cont const& cont) {
    return as_cidx(size(cont));
}

// Since ridx_t could be any integer type, rsize provide a (debug) checked way to get the size of a
// container as ridx_t. It also allows to avoid narrowing conversion warnings.
template <typename Cont>
inline ridx_t rsize(Cont const& cont) {
    return as_ridx(size(cont));
}

}  // namespace cft


#endif /* CFT_SRC_CORE_CFT_HPP */
