// SPDX-FileCopyrightText: 2024 Francesco Cavaliere <francescocava95@gmail.com>
// SPDX-License-Identifier: MIT

#ifndef CFT_SRC_CORE_SORT_HPP
#define CFT_SRC_CORE_SORT_HPP


#include <algorithm>
#include <cstddef>

#include "utils/utility.hpp"

namespace cft {

// Hook for future specializations
template <typename C, typename K = IdentityFtor>
void nth_element(C& container, size_t nth_elem, K key = {}) {
    using value_type = container_value_type_t<C>;
    std::nth_element(container.begin(),
                     container.begin() + checked_cast<std::ptrdiff_t>(nth_elem),
                     container.end(),
                     [key](value_type const& a, value_type const& b) { return key(a) < key(b); });
}

// Hook for future specializations
template <typename C, typename K = IdentityFtor>
void sort(C& container, K key = {}) {
    using value_type = container_value_type_t<C>;
    std::sort(container.begin(), container.end(), [key](value_type const& a, value_type const& b) {
        return key(a) < key(b);
    });
}

}  // namespace cft


#endif /* CFT_SRC_CORE_SORT_HPP */
