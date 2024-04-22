// SPDX-FileCopyrightText: 2024 Francesco Cavaliere <francescocava95@gmail.com>
// SPDX-License-Identifier: MIT

#ifndef CFT_SRC_CORE_LIMITS_HPP
#define CFT_SRC_CORE_LIMITS_HPP


#include <limits>

namespace cft {

// Syntactic sugar for numeric max and min values.  It can be extended to defined max and min for
// other types.
template <typename T>
struct limits {
    static constexpr T max() {
        return std::numeric_limits<T>::max();
    }

    static constexpr T min() {
        return std::numeric_limits<T>::lowest();
    }

    static constexpr T inf() {
        return std::numeric_limits<T>::infinity();
    }
};

}  // namespace cft


#endif /* CFT_SRC_CORE_LIMITS_HPP */
