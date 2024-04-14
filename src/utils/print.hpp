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

#ifndef CAV_SRC_UTILS_PRINT_HPP
#define CAV_SRC_UTILS_PRINT_HPP

#include <fmt/core.h>
#include <fmt/ranges.h>

#include "core/cft.hpp"

namespace cft {

// Print a message to stdout if the verbosity level is greater or equal to Level
template <uint64_t Level, typename... T>
inline void print(Environment const& env, fmt::format_string<T...> fmt, T&&... args) {
    if (env.verbose >= Level)
        fmt::print(fmt, std::forward<T>(args)...);
}

template <uint64_t Level, typename... T>
inline void print(Environment const& env, FILE* file, fmt::format_string<T...> fmt, T&&... args) {
    if (env.verbose >= Level)
        fmt::print(file, fmt, std::forward<T>(args)...);
}

template <typename... T>
inline void eprint(fmt::format_string<T...> fmt, T&&... args) {
    fmt::print(stderr, fmt, std::forward<T>(args)...);
}


}  // namespace cft

#endif /* CAV_SRC_UTILS_PRINT_HPP */
