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

namespace cft {

template <typename T, typename LT, typename UT>
T clamp(T const& v, LT const& lb, UT const& ub) noexcept {
    return (v < lb) ? static_cast<T>(lb) : (v > ub) ? static_cast<T>(ub) : v;
}

template <typename T>
T abs(T val) noexcept {
    return val < T{} ? -val : val;
}

/// Multi-arg max
template <typename T>
T max(T v) noexcept {
    return v;
}

template <typename T1, typename T2, typename... Ts>
T1 max(T1 v1, T2 v2, Ts... tail) noexcept {
    T1 mtail = max<T1>(v2, tail...);
    return (v1 >= mtail ? v1 : mtail);
}

template <typename... Ts>
bool max(bool b1, bool b2, Ts... tail) noexcept {
    return b1 || max(b2, tail...);
}

/// Multi-arg min
template <typename T>
T min(T v) noexcept {
    return v;
}

template <typename T1, typename T2, typename... Ts>
T1 min(T1 v1, T2 v2, Ts... tail) noexcept {
    T1 mtail = min<T1>(v2, tail...);
    return v1 <= mtail ? v1 : mtail;
}

template <typename... Ts>
bool min(bool b1, bool b2, Ts... tail) noexcept {
    return b1 && min(b2, tail...);
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
