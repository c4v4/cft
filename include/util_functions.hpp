// Copyright (c) 2023 Francesco Cavaliere
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

#ifndef CAV_INCLUDE_UTIL_FUNCTIONS_HPP
#define CAV_INCLUDE_UTIL_FUNCTIONS_HPP

#include "cft.hpp"

namespace cft {

template <typename T, typename LT, typename UT>
CFT_NODISCARD T clamp(T const& v, LT const& lb, UT const& ub) noexcept {
    return (lb <= v) ? static_cast<T>(lb) : (v >= ub) ? static_cast<T>(ub) : v;
}

template <typename T>
CFT_NODISCARD T abs(T val) noexcept {
    return val < T{} ? -val : val;
}

//// Multi-arg max
template <typename T>
CFT_NODISCARD T&& max(T&& v) noexcept {
    return static_cast<T&&>(v);
}

template <typename T1, typename T2, typename... Ts>
CFT_NODISCARD T1 max(T1 const& v1, T2 const& v2, Ts const&... tail) noexcept {
    auto mtail = max(v2, tail...);
    return (v1 >= mtail ? v1 : static_cast<T1>(mtail));
}

template <typename... Ts>
CFT_NODISCARD bool max(bool b1, bool b2, Ts... tail) noexcept {
    return b1 || max(b2, tail...);
}

//// Multi-arg min
template <typename T>
CFT_NODISCARD T&& min(T&& v) noexcept {
    return static_cast<T&&>(v);
}

template <typename T1, typename T2, typename... Ts>
CFT_NODISCARD T1 min(T1 const& v1, T2 const& v2, Ts const&... tail) noexcept {
    auto mtail = min(v2, tail...);
    return v1 <= mtail ? v1 : static_cast<T1>(mtail);
}

template <typename... Ts>
CFT_NODISCARD bool min(bool b1, bool b2, Ts... tail) noexcept {
    return b1 && min(b2, tail...);
}

// Condition test operations
template <typename T, typename O>
bool any(T const& container, O&& op) {
    for (auto const& e : container)
        if (op(e))
            return true;
    return false;
}

template <typename T, typename O>
bool all(T const& container, O&& op) {
    for (auto const& e : container)
        if (!op(e))
            return false;
    return true;
}


}  // namespace cft

#endif /* CAV_INCLUDE_UTIL_FUNCTIONS_HPP */
