// SPDX-FileCopyrightText: 2024 Francesco Cavaliere <francescocava95@gmail.com>
// SPDX-License-Identifier: MIT

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#define DOCTEST_CONFIG_SUPER_FAST_ASSERTS
#include <doctest/doctest.h>

#include "utils/parse_utils.hpp"

namespace cft {


TEST_CASE("ltrim: should remove leading spaces") {
    auto str = std::string("   hello");
    CHECK(ltrim(str) == "hello");
}

TEST_CASE("ltrim: should not modify string if no leading spaces") {
    auto str = std::string("hello");
    CHECK(ltrim(str) == "hello");
}

TEST_CASE("rtrim: should remove trailing spaces") {
    auto str = std::string("hello   ");
    CHECK(rtrim(str) == "hello");
}

TEST_CASE("rtrim: should not modify string if no trailing spaces") {
    auto str = std::string("hello");
    CHECK(rtrim(str) == "hello");
}

TEST_CASE("trim: should remove leading and trailing spaces") {
    auto str = std::string("   hello   ");
    CHECK(trim(str) == "hello");
}

TEST_CASE("trim: should not modify string if no leading or trailing spaces") {
    auto str = std::string("hello");
    CHECK(trim(str) == "hello");
}

TEST_CASE("split: should split string into vector of substrings") {
    auto str      = std::string("hello world");
    auto expected = std::vector<StringView>{"hello", "world"};
    CHECK(split(str) == expected);
}

TEST_CASE("split: should handle multiple spaces between substrings") {
    auto str      = std::string("hello   world");
    auto expected = std::vector<StringView>{"hello", "world"};
    CHECK(split(str) == expected);
}

TEST_CASE("split: should handle leading and trailing spaces") {
    auto str      = std::string("   hello   world   ");
    auto expected = std::vector<StringView>{"hello", "world"};
    CHECK(split(str) == expected);
}

TEST_CASE("string_to: should parse string to integral types") {
    auto str = std::string("123");
    CHECK(string_to<int>::parse(str) == 123);
    CHECK(string_to<long>::parse(str) == 123);
    CHECK(string_to<long long>::parse(str) == 123);
}

TEST_CASE("string_to: should parse string to floating-point types") {
    auto str = std::string("3.14");
    CHECK(string_to<float>::parse(str) == doctest::Approx(3.14F));
    CHECK(string_to<double>::parse(str) == doctest::Approx(3.14));
    CHECK(string_to<long double>::parse(str) == doctest::Approx(3.14L));
}

}  // namespace cft