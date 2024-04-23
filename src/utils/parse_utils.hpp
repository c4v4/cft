// SPDX-FileCopyrightText: 2024 Francesco Cavaliere <francescocava95@gmail.com>
// SPDX-License-Identifier: MIT

#ifndef CFT_SRC_CORE_PARSE_UTILS_HPP
#define CFT_SRC_CORE_PARSE_UTILS_HPP


#include <fmt/format.h>

#include <cerrno>
#include <cstddef>
#include <fstream>
#include <stdexcept>
#include <type_traits>
#include <vector>

#include "utils/StringView.hpp"
#include "utils/limits.hpp"
#include "utils/utility.hpp"

namespace cft {

struct IsSpace {
    bool operator()(char c) const {
        return std::isspace(c) != 0;
    }
};

struct NotSpace {
    bool operator()(char c) const {
        return std::isspace(c) == 0;
    }
};

inline StringView ltrim(StringView str) {
    size_t prefix = str.find_first_true(NotSpace{});
    return str.remove_prefix(prefix);
}

inline StringView rtrim(StringView str) {
    if (str.empty())
        return str;
    size_t suffix = str.find_last_true(NotSpace{});
    return str.remove_suffix(suffix + 1);
}

inline StringView trim(StringView str) {
    return rtrim(ltrim(str));
}

inline std::vector<StringView> split(StringView str) {
    auto trimmed = trim(str);
    auto elems   = std::vector<StringView>{};
    while (!trimmed.empty()) {
        size_t elem_end = trimmed.find_first_true(IsSpace{});
        elems.push_back(trimmed.get_substr(0, elem_end));
        trimmed = ltrim(trimmed.remove_prefix(elem_end));
    }
    return elems;
}

// Parse a string to a specific native scalar type
template <typename T>
struct string_to {
    using val_t = native_t<T>;
    static_assert(std::is_integral<val_t>::value || std::is_floating_point<val_t>::value,
                  "Only integral and floating point types are supported");

    template <bool C, typename T1, typename T2>
    using if_t = typename std::conditional<C, T1, T2>::type;

    using safe_type = if_t<std::is_integral<val_t>::value,
                           if_t<std::is_signed<val_t>::value, int64_t, uint64_t>,
                           long double>;

    // Parse without modifying the string view  (note the pass by value)
    static T parse(StringView str) {
        return consume(str);
    }

    // Parse and modify the string view removing the element parsed
    static T consume(StringView& str) {
        if (str.empty())
            throw std::invalid_argument(
                fmt::format("Invalid argument parsing {}", typeid(T).name()));

        safe_type val      = {};
        bool      oo_range = false;  // out of range
        char*     end      = nullptr;
        errno              = 0;

        if (std::is_floating_point<val_t>::value) {
            val      = std::strtold(str.data(), &end);
            oo_range = !std::isfinite(val);
        } else if (std::is_signed<val_t>::value) {
            val      = std::strtoll(str.data(), &end, 10);
            oo_range = val == limits<val_t>::max() && errno == ERANGE;
        } else {  // unsigned
            val      = std::strtoull(str.data(), &end, 10);
            oo_range = val == limits<val_t>::max() && errno == ERANGE;
        }

        if (oo_range || val < limits<val_t>::min() || limits<val_t>::max() < val)
            throw std::out_of_range(
                fmt::format("Out of range parsing {} (as {})", str.data(), typeid(T).name()));

        if (end == str.data())
            throw std::invalid_argument(
                fmt::format("Invalid argument parsing {} (as {})", str.data(), typeid(T).name()));
        str = str.remove_prefix(checked_cast<size_t>(end - str.data()));

        return checked_cast<T>(val);
    }
};

// Reads a file line by line returning trimmed string views
struct FileLineIterator {
    std::ifstream in;
    std::string   line;

    explicit FileLineIterator(std::string const& path)
        : in(path) {
        if (!in.is_open())
            throw std::invalid_argument(fmt::format("Cannot open file {}", path));
    }

    explicit FileLineIterator(char const* path)
        : in(path) {
        if (!in.is_open())
            throw std::invalid_argument(fmt::format("Cannot open file {}", path));
    }

    StringView next() {
        std::getline(in, line);
        return trim(StringView(line));
    }
};

}  // namespace cft


#endif /* CFT_SRC_CORE_PARSE_UTILS_HPP */
