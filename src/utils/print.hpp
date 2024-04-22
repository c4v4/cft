// SPDX-FileCopyrightText: 2024 Francesco Cavaliere <francescocava95@gmail.com>
// SPDX-License-Identifier: MIT

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

}  // namespace cft

#endif /* CAV_SRC_UTILS_PRINT_HPP */
