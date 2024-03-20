// Copyright (c) 2024 Francesco Cavaliere
// This program is free software: you can redistribute it and/or modify it under the terms of the
// GNU General Public License as published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version. This program is distributed in the hope that it
// will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details. You should
// have received a copy of the GNU General Public License along with this program. If not, see
// <https://www.gnu.org/licenses/>.

#ifndef CFT_INCLUDE_RADIX_STUFF_HPP
#define CFT_INCLUDE_RADIX_STUFF_HPP

#include <algorithm>
#include <utility>

namespace cft {

namespace {
    template <typename C>
    using container_iterator_t = decltype(std::declval<C>().begin());
    template <typename C>
    using container_value_type_t = decltype(*std::declval<container_iterator_t<C>>());
    template <typename C>
    using container_size_type_t = decltype(std::declval<C>().size());
}  // namespace

struct IdentityFtor {
    template <typename T>
    T&& operator()(T&& t) const noexcept {
        return std::forward<T>(t);
    }
};

struct Sorter {
    // Temporary implementation with fallback to std::nth_element
    template <typename C, typename K = IdentityFtor>
    void nth_element(C& container, size_t nth_elem, K key = {}) {
        using value_type = container_value_type_t<C>;
        std::nth_element(container.begin(),
                         container.begin() + nth_elem,
                         container.end(),
                         [&](value_type const& a, value_type const& b) { return key(a) < key(b); });
    }

    // Temporary implementation with fallback to std::sort
    template <typename C, typename K = IdentityFtor>
    void sort(C& container, K key = {}) {
        using value_type = container_value_type_t<C>;
        std::sort(container.begin(),
                  container.end(),
                  [&](value_type const& a, value_type const& b) { return key(a) < key(b); });
    }
};

}  // namespace cft

#endif /* CFT_INCLUDE_RADIX_STUFF_HPP */
