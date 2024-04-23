// SPDX-FileCopyrightText: 2024 Francesco Cavaliere <francescocava95@gmail.com>
// SPDX-License-Identifier: MIT

#ifndef CFT_SRC_CORE_LIMITS_HPP
#define CFT_SRC_CORE_LIMITS_HPP

#include <limits>

#include "utils/custom_types.hpp"

namespace cft {

// Syntactic sugar for numeric max and min values.  It can be extended to defined max and min for
// other types.
template <typename T>
struct limits {
    static constexpr T max() {
        return T{std::numeric_limits<native_t<T>>::max()};
    }

    static constexpr T min() {
        return T{std::numeric_limits<native_t<T>>::lowest()};
    }

    static constexpr T inf() {
        return T{std::numeric_limits<native_t<T>>::infinity()};
    }
};

}  // namespace cft


#endif /* CFT_SRC_CORE_LIMITS_HPP */
