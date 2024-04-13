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