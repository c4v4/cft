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
