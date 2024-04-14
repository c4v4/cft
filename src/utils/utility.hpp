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

#ifndef CFT_SRC_CORE_UTILITY_HPP
#define CFT_SRC_CORE_UTILITY_HPP


#include <cstddef>
#include <type_traits>

#include "utils/assert.hpp"  // IWYU pragma: keep
#include "utils/limits.hpp"

namespace cft {


////////////////////////////////////// CHECKED CAST //////////////////////////////////////
// We want to wrap static_cast for numeric values, checking that some properties are satisfied:
// 1. If we start from an integral type, the round-trip is lossless.
// 2. The sign is preserved between integrals casts.
// 3. The value is in range for the target type for floating point casts.
//
// NOTE: this is NOT a `narrow-cast`, we might lose information with floating point values.

// SFINAE helpers, activate overload if T is integral or floating point
template <typename T>
using requires_integral = typename std::enable_if<std::is_integral<T>::value, bool>::type;
template <typename T>
using requires_floating_point = typename std::enable_if<std::is_floating_point<T>::value,
                                                        bool>::type;

// NOTE: integral -> floating point must be in the range where the floating point can represent the
// integral with precision at least 1
template <typename ResT, typename T, requires_integral<T> = true>
constexpr ResT checked_cast(T val) {
    return assert(static_cast<T>(static_cast<ResT>(val)) == val),             // Lossless round-trip
           assert(std::is_signed<T>::value == std::is_signed<ResT>::value ||  // Sign match
                  (static_cast<ResT>(val) < ResT{}) == (val < T{})),
           static_cast<ResT>(val);
}

template <typename ResT, typename T, requires_floating_point<T> = true>
constexpr ResT checked_cast(T val) {
    // Here ResT can be integral, if val in in range, then any floating point can be casted to it
    return assert(limits<ResT>::min() <= val && val <= limits<ResT>::max()),  // Range check
           static_cast<ResT>(val);
}

template <typename T, typename LT, typename UT>
T clamp(T const& v, LT const& lb, UT const& ub) {
    return (v < lb) ? static_cast<T>(lb) : (v > ub) ? static_cast<T>(ub) : v;
}

template <typename T>
T abs(T val) {
    return val < T{} ? -val : val;
}

/// Multi-arg max. NOTE: to avoid ambiguity, return type is always the first argument type
template <typename T>
T max(T v) {
    return v;
}

template <typename T1, typename T2, typename... Ts>
T1 max(T1 v1, T2 v2, Ts... tail) {
    T1 mtail = max<T1>(v2, tail...);
    return (v1 >= mtail ? v1 : mtail);
}

/// Multi-arg min. NOTE: to avoid ambiguity, return type is always the first argument type
template <typename T>
T min(T v) {
    return v;
}

template <typename T1, typename T2, typename... Ts>
T1 min(T1 v1, T2 v2, Ts... tail) {
    T1 mtail = min<T1>(v2, tail...);
    return v1 <= mtail ? v1 : mtail;
}

// Condition test operations
template <typename T, typename O>
bool any(T const& container, O op) {
    for (auto const& e : container)
        if (op(e))
            return true;
    return false;
}

template <typename T, typename O>
bool all(T const& container, O op) {
    for (auto const& e : container)
        if (!op(e))
            return false;
    return true;
}

template <typename C>
size_t size(C const& container) {
    return container.size();
}

template <typename C, size_t N>
constexpr size_t size(C const (& /*unused*/)[N]) {
    return N;
}

template <typename C, typename K>
size_t argmin(C const& container, K key) {
    size_t min_i = 0;
    for (size_t i = 1; i < size(container); ++i)
        min_i = key(container[i]) < key(container[min_i]) ? i : min_i;
    return min_i;
}

template <typename C, typename Op>
void remove_if(C& container, Op op) {
    size_t w = 0;
    for (size_t r = 0; r < size(container); ++r)
        if (!op(container[r]))
            container[w++] = container[r];
    container.resize(w);
}


}  // namespace cft


#endif /* CFT_SRC_CORE_UTILITY_HPP */
