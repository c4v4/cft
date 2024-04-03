// Copyright (c) 2024 Luca Accorsi and Francesco Cavaliere
// This program is free software: you can redistribute it and/or modify it under the terms of the
// GNU General Public License as published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version. This program is distributed in the hope that it
// will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details. You should
// have received a copy of the GNU General Public License along with this program. If not, see
// <https://www.gnu.org/licenses/>.

#ifndef CFT_SRC_CORE_PARSEUTILS_HPP
#define CFT_SRC_CORE_PARSEUTILS_HPP

#include <fmt/format.h>

#include <cerrno>
#include <cstddef>
#include <fstream>
#include <stdexcept>
#include <type_traits>

#include "core/StringView.hpp"
#include "core/limits.hpp"

namespace cft {

template <bool C, typename T1, typename T2>
using if_t = typename std::conditional<C, T1, T2>::type;

struct IsSpace {
    CFT_NODISCARD bool operator()(char c) const noexcept {
        return std::isspace(c) != 0;
    }
};

struct NotSpace {
    CFT_NODISCARD bool operator()(char c) const noexcept {
        return std::isspace(c) == 0;
    }
};

inline StringView ltrim(StringView str) {
    size_t prefix = str.find_first_if(NotSpace{});
    return str.remove_prefix(prefix);
}

inline StringView rtrim(StringView str) {
    size_t suffix = str.find_last_if(NotSpace{});
    return str.remove_suffix(suffix + 1);
}

inline StringView trim(StringView str) {
    return rtrim(ltrim(str));
}

inline std::vector<StringView> split(StringView str) {
    auto trimmed = trim(str);
    auto elems   = std::vector<StringView>{};
    while (!trimmed.empty()) {
        size_t end = str.find_first_if(IsSpace{});
        elems.push_back(trimmed.get_substr(0, end));
        trimmed = ltrim(trimmed.remove_prefix(end));
    }
    return elems;
}

template <typename T>
struct string_to {
    using safe_type = if_t<std::is_integral<T>::value,
                           if_t<std::is_signed<T>::value, int64_t, uint64_t>,
                           long double>;

    // parse without modifying the string view
    static T parse(StringView str) {
        return consume(str);
    }

    // parse and modify the string view removing the element parsed
    static T consume(StringView& str) {
        if (str.empty())
            throw std::invalid_argument(
                fmt::format("Invalid argument parsing {}", typeid(T).name()));

        safe_type val      = {};
        bool      oo_range = false;
        char*     end      = nullptr;
        errno              = 0;

        if (!std::is_integral<T>::value) {
            val      = std::strtold(str.data(), &end);
            oo_range = val == limits<T>::inf();
        } else if (std::is_signed<T>::value) {
            val      = std::strtoll(str.data(), &end, 10);
            oo_range = val == limits<T>::max() && errno == ERANGE;
        } else {
            val      = std::strtoull(str.data(), &end, 10);
            oo_range = val == limits<T>::max() && errno == ERANGE;
        }

        if (oo_range || val > limits<T>::max() || val < limits<T>::min())
            throw std::out_of_range(fmt::format("Our of range parsing {}", typeid(T).name()));

        if (end == str.data())
            throw std::invalid_argument(
                fmt::format("Invalid argument parsing (typeid: {})", typeid(T).name()));
        str = str.remove_prefix(end - str.data());

        return static_cast<T>(val);
    }
};

struct FileLineIterator {
    std::ifstream in;
    std::string   line;

    FileLineIterator(std::string const& path)
        : in(path) {
    }

    FileLineIterator(char const* path)
        : in(path) {
    }

    CFT_NODISCARD StringView next() {
        std::getline(in, line);
        return trim(StringView(line));
    }
};

}  // namespace cft

#endif /* CFT_SRC_CORE_PARSEUTILS_HPP */
