// SPDX-FileCopyrightText: 2024 Francesco Cavaliere <francescocava95@gmail.com>
// SPDX-License-Identifier: MIT

#define CATCH_CONFIG_MAIN 
#include <catch2/catch.hpp>

#include "utils/parse_utils.hpp"

namespace cft {

TEST_CASE("ltrim") {
    SECTION("should remove leading spaces") {
        auto str = std::string("   hello");
        REQUIRE(ltrim(str) == "hello");
    }

    SECTION("should not modify string if no leading spaces") {
        auto str = std::string("hello");
        REQUIRE(ltrim(str) == "hello");
    }
}

TEST_CASE("rtrim") {
    SECTION("should remove trailing spaces") {
        auto str = std::string("hello   ");
        REQUIRE(rtrim(str) == "hello");
    }

    SECTION("should not modify string if no trailing spaces") {
        auto str = std::string("hello");
        REQUIRE(rtrim(str) == "hello");
    }
}

TEST_CASE("trim") {
    SECTION("should remove leading and trailing spaces") {
        auto str = std::string("   hello   ");
        REQUIRE(trim(str) == "hello");
    }

    SECTION("should not modify string if no leading or trailing spaces") {
        auto str = std::string("hello");
        REQUIRE(trim(str) == "hello");
    }
}

TEST_CASE("split") {
    SECTION("should split string into vector of substrings") {
        auto str      = std::string("hello world");
        auto expected = std::vector<StringView>{"hello", "world"};
        REQUIRE(split(str) == expected);
    }

    SECTION("should handle multiple spaces between substrings") {
        auto str      = std::string("hello   world");
        auto expected = std::vector<StringView>{"hello", "world"};
        REQUIRE(split(str) == expected);
    }

    SECTION("should handle leading and trailing spaces") {
        auto str      = std::string("   hello   world   ");
        auto expected = std::vector<StringView>{"hello", "world"};
        REQUIRE(split(str) == expected);
    }
}

TEST_CASE("string_to") {
    SECTION("should parse string to integral types") {
        auto str = std::string("123");
        REQUIRE(string_to<int>::parse(str) == 123);
        REQUIRE(string_to<long>::parse(str) == 123);
        REQUIRE(string_to<long long>::parse(str) == 123);
    }

    SECTION("should parse string to floating-point types") {
        auto str = std::string("3.14");
        REQUIRE(string_to<float>::parse(str) == Approx(3.14F));
        REQUIRE(string_to<double>::parse(str) == Approx(3.14));
        REQUIRE(string_to<long double>::parse(str) == Approx(3.14L));
    }
}

}  // namespace cft