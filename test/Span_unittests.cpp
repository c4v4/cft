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

#include "utils/Span.hpp"

namespace cft {

TEST_CASE("Span size and empty") {
    int  arr[] = {1, 2, 3, 4, 5};
    auto span  = make_span(arr, 5);

    SECTION("Size of span") {
        REQUIRE(span.size() == 5);
    }

    SECTION("Empty span") {
        REQUIRE_FALSE(span.empty());
    }
}

TEST_CASE("Span back") {
    int  arr[] = {1, 2, 3, 4, 5};
    auto span  = make_span(arr, 5);

    SECTION("Back element of span") {
        REQUIRE(span.back() == 5);
    }
}

TEST_CASE("Span iteration") {
    int  arr[] = {1, 2, 3, 4, 5};
    auto span  = make_span(arr, 5);

    SECTION("Iterating over span") {
        int sum = 0;
        for (auto& element : span)
            sum += element;
        REQUIRE(sum == 15);
    }
}

TEST_CASE("Span indexing") {
    int  arr[] = {1, 2, 3, 4, 5};
    auto span  = make_span(arr, 5);

    SECTION("Indexing span") {
        REQUIRE(span[0] == 1);
        REQUIRE(span[2] == 3);
        REQUIRE(span[4] == 5);
    }
}

TEST_CASE("Make span") {
    int arr[] = {1, 2, 3, 4, 5};

    SECTION("Make span with iterators") {
        auto span = make_span(std::begin(arr), std::end(arr));
        REQUIRE(span.size() == 5);
        REQUIRE(span[0] == 1);
        REQUIRE(span[4] == 5);
    }

    SECTION("Make span with size") {
        auto span = make_span(std::begin(arr), 3);
        REQUIRE(span.size() == 3);
        REQUIRE(span[0] == 1);
        REQUIRE(span[2] == 3);
    }
}

#ifndef NDEBUG

TEST_CASE("Test Span assert fails") {
    int base[] = {1, 2, 3, 4, 5};
    REQUIRE_THROWS_AS(make_span(base + 5, base), std::runtime_error);
    auto illegal_span = Span<int*>{base + 5, base};
    REQUIRE_THROWS_AS(illegal_span.empty(), std::runtime_error);
    REQUIRE_THROWS_AS(illegal_span.size(), std::runtime_error);
    REQUIRE_THROWS_AS(illegal_span.back(), std::runtime_error);
    REQUIRE_THROWS_AS(illegal_span.begin(), std::runtime_error);
    REQUIRE_THROWS_AS(illegal_span.end(), std::runtime_error);

    auto span = make_span(base, 5);
    REQUIRE_THROWS_AS(span[5], std::runtime_error);
}

#endif

}  // namespace cft
