// SPDX-FileCopyrightText: 2024 Francesco Cavaliere <francescocava95@gmail.com>
// SPDX-License-Identifier: MIT

#ifndef CAV_SRC_UTILS_PRINT_HPP
#define CAV_SRC_UTILS_PRINT_HPP

#include <fmt/core.h>
#include <fmt/ranges.h>

#include "core/cft.hpp"
#include "utils/custom_types.hpp"

// Custom formatter for any type that defines a native type
namespace fmt {
template <typename T>
struct formatter<T, char, typename std::enable_if<!std::is_same<T, cft::native_t<T>>::value>::type>
    : fmt::formatter<cft::native_t<T>> {
    using nat_t       = cft::native_t<T>;
    using base        = fmt::formatter<nat_t>;
    using tagged_type = T;

    auto format(tagged_type c, format_context& ctx) const noexcept
        -> decltype(base::format(static_cast<nat_t>(c), ctx)) {
        return base::format(static_cast<nat_t>(c), ctx);
    }
};
}  // namespace fmt

namespace cft {

// Print a message to stdout if the verbosity level is greater or equal to Level
template <uint64_t Level, typename... T>
inline void print(Environment const& env, fmt::format_string<T...> fmt, T&&... args) {
    if (env.verbose >= Level)
        fmt::print(fmt, std::forward<T>(args)...);
}

}  // namespace cft

#endif /* CAV_SRC_UTILS_PRINT_HPP */
