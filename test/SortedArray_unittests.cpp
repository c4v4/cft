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

#include "utils/SortedArray.hpp"

namespace cft {

TEST_CASE("test_sorted_sequence") {
    auto arr = SortedArray<int, 5>();
    arr.insert(1);
    arr.insert(2);
    arr.insert(3);
    arr.insert(4);
    arr.insert(5);

    REQUIRE(arr.size() == 5);
    REQUIRE(arr.back() == 5);

    int expected[] = {1, 2, 3, 4, 5};
    for (size_t i = 0; i < arr.size(); ++i)
        REQUIRE(arr[i] == expected[i]);
    int prev = -1;
    for (int elem : arr) {
        REQUIRE(prev < elem);
        prev = elem;
    }
}

TEST_CASE("test_unsorted_sequence") {
    auto arr = SortedArray<int, 5>();
    arr.insert(3);
    arr.insert(1);
    arr.insert(5);
    arr.insert(2);
    arr.insert(4);

    REQUIRE(arr.size() == 5);
    REQUIRE(arr.back() == 5);

    int expected[] = {1, 2, 3, 4, 5};
    for (size_t i = 0; i < arr.size(); ++i)
        REQUIRE(arr[i] == expected[i]);
    int prev = -1;
    for (int elem : arr) {
        REQUIRE(prev < elem);
        prev = elem;
    }
}

TEST_CASE("test_empty_sequence") {
    auto arr = SortedArray<int, 5>();

    REQUIRE(arr.size() == 0);
    REQUIRE(arr.begin() == arr.end());
}

TEST_CASE("test_ovefull_sequence") {
    auto arr = SortedArray<int, 5>();
    arr.insert(3);
    arr.insert(6);
    arr.insert(1);
    arr.insert(9);
    arr.insert(2);
    arr.try_insert(4);
    arr.try_insert(5);
    arr.try_insert(7);
    arr.try_insert(10);
    arr.try_insert(8);

    REQUIRE(arr.size() == 5);
    REQUIRE(arr.back() == 5);

    int expected[] = {1, 2, 3, 4, 5};
    for (size_t i = 0; i < arr.size(); ++i)
        REQUIRE(arr[i] == expected[i]);
    int prev = -1;
    for (int elem : arr) {
        REQUIRE(prev < elem);
        prev = elem;
    }
}

TEST_CASE("test_custom_compare") {
    int  idx_array[] = {4, 3, 2, 1, 0};
    auto arr         = make_custom_compare_sorted_array<int, 5>(
        [&](int a, int b) { return idx_array[a] < idx_array[b]; });
    arr.insert(4);
    arr.insert(0);
    arr.insert(2);
    arr.insert(1);
    arr.insert(3);

    REQUIRE(arr.size() == 5);
    REQUIRE(arr.back() == 0);

    for (size_t i = 0; i < arr.size(); ++i)
        REQUIRE(arr[i] == idx_array[i]);
    int prev = -1;
    for (int elem : arr) {
        REQUIRE(prev < idx_array[elem]);
        prev = idx_array[elem];
    }
}

#ifndef NDEBUG

TEST_CASE("Test SortedArray assert fails") {
    auto arr = SortedArray<int, 5>();

    REQUIRE_THROWS_AS(arr.back(), std::runtime_error);
    arr.insert(1);
    REQUIRE(arr.back() == 1);
    REQUIRE(arr[0] == 1);
    REQUIRE_THROWS_AS(arr[1], std::runtime_error);
}

#endif

}  // namespace cft
