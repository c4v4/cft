// SPDX-FileCopyrightText: 2024 Francesco Cavaliere <francescocava95@gmail.com>
// SPDX-License-Identifier: MIT

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#define DOCTEST_CONFIG_SUPER_FAST_ASSERTS
 
#include <doctest/doctest.h>

#include "utils/SortedArray.hpp"

namespace cft {

TEST_CASE("test_sorted_sequence") {
    auto arr = SortedArray<int, 5>();
    arr.insert(1);
    arr.insert(2);
    arr.insert(3);
    arr.insert(4);
    arr.insert(5);

    CHECK(arr.size() == 5);
    CHECK(arr.back() == 5);

    int expected[] = {1, 2, 3, 4, 5};
    for (size_t i = 0; i < arr.size(); ++i)
        CHECK(arr[i] == expected[i]);
    int prev = -1;
    for (int elem : arr) {
        CHECK(prev < elem);
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

    CHECK(arr.size() == 5);
    CHECK(arr.back() == 5);

    int expected[] = {1, 2, 3, 4, 5};
    for (size_t i = 0; i < arr.size(); ++i)
        CHECK(arr[i] == expected[i]);
    int prev = -1;
    for (int elem : arr) {
        CHECK(prev < elem);
        prev = elem;
    }
}

TEST_CASE("test_empty_sequence") {
    auto arr = SortedArray<int, 5>();

    CHECK(arr.size() == 0);
    CHECK(arr.begin() == arr.end());
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

    CHECK(arr.size() == 5);
    CHECK(arr.back() == 5);

    int expected[] = {1, 2, 3, 4, 5};
    for (size_t i = 0; i < arr.size(); ++i)
        CHECK(arr[i] == expected[i]);
    int prev = -1;
    for (int elem : arr) {
        CHECK(prev < elem);
        prev = elem;
    }
}

TEST_CASE("test_custom_key") {
    int  idx_array[] = {4, 3, 2, 1, 0};
    auto arr         = make_custom_key_sorted_array<int, 5>([&](int a) { return idx_array[a]; });
    arr.insert(4);
    arr.insert(0);
    arr.insert(2);
    arr.insert(1);
    arr.insert(3);

    CHECK(arr.size() == 5);
    CHECK(arr.back() == 0);

    for (size_t i = 0; i < arr.size(); ++i)
        CHECK(arr[i] == idx_array[i]);
    int prev = -1;
    for (int elem : arr) {
        CHECK(prev < idx_array[elem]);
        prev = idx_array[elem];
    }
}

#ifndef NDEBUG

TEST_CASE("Test SortedArray assert fails") {
    auto arr = SortedArray<int, 5>();

    CHECK_THROWS_AS(arr.back(), std::runtime_error);
    arr.insert(1);
    CHECK(arr.back() == 1);
    CHECK(arr[0] == 1);
    CHECK_THROWS_AS(arr[1], std::runtime_error);
}

#endif

}  // namespace cft
