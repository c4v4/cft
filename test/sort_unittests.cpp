// SPDX-FileCopyrightText: 2024 Francesco Cavaliere <francescocava95@gmail.com>
// SPDX-License-Identifier: MIT

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#define DOCTEST_CONFIG_SUPER_FAST_ASSERTS
#include <doctest/doctest.h>

#include <vector>

#include "core/cft.hpp"
#include "utils/random.hpp"
#include "utils/sort.hpp"

TEST_CASE("Sort an empty container") {
    auto container = std::vector<int>{};
    cft::sort(container);
    CHECK(container.empty());
}

TEST_CASE("Sort a container with one element") {
    auto container = std::vector<int>{5};
    cft::sort(container);
    CHECK(container == std::vector<int>{5});
}

TEST_CASE("Sort a container with multiple elements") {
    auto container = std::vector<int>{5, 2, 8, 1, 9};
    cft::sort(container);
    CHECK(container == std::vector<int>{1, 2, 5, 8, 9});
}

TEST_CASE("Sort container with 10000 random number and check it is sorted") {
    auto container = std::vector<int>{};
    auto rnd       = cft::prng_t{};
    for (size_t i = 0; i < 10000; ++i)
        container.push_back(cft::roll_dice(rnd, -1000, 1000));
    cft::sort(container);
    CHECK(std::is_sorted(container.begin(), container.end()));
}

TEST_CASE("nth_element on an empty container") {
    auto container = std::vector<int>{};
    cft::nth_element(container, 0);
    CHECK(container.empty());
}

TEST_CASE("nth_element on a container with one element") {
    auto container = std::vector<int>{5};
    cft::nth_element(container, 0);
    CHECK(container == std::vector<int>{5});
}

TEST_CASE("nth_element on a container with multiple elements") {
    auto container = std::vector<int>{5, 2, 8, 1, 9};
    cft::nth_element(container, 2);
    CHECK(container[2] == 5);
}

TEST_CASE("nth_element on a container with duplicate elements") {
    auto container = std::vector<int>{1, 2, 1, 1, 2};
    cft::nth_element(container, 2);
    CHECK(container[2] == 1);
}
