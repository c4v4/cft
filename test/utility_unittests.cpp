// SPDX-FileCopyrightText: 2024 Francesco Cavaliere <francescocava95@gmail.com>
// SPDX-License-Identifier: MIT

#include <array>
#include <cstdint>
#include <vector>
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#define DOCTEST_CONFIG_SUPER_FAST_ASSERTS
#include <doctest/doctest.h>

#ifndef NDEBUG
#include <stdexcept>
#endif

#include "utils/utility.hpp"

namespace cft {

TEST_CASE("checked_cast - Test with integers") {
    SUBCASE("checked_cast - Positive to smaller positive") {
        auto result = checked_cast<int16_t>(42);
        CHECK(result == 42);
    }

    SUBCASE("checked_cast - Negative to smaller negative") {
        auto result = checked_cast<int16_t>(-42);
        CHECK(result == -42);
    }

    SUBCASE("checked_cast - Float to int") {
        float value = 3.14F;
        CHECK(checked_cast<int16_t>(value) == 3);
    }

    SUBCASE("checked_cast - Double to int") {
        double value = 1212.14;
        CHECK(checked_cast<int16_t>(value) == 1212);
    }
}

#ifndef NDEBUG

TEST_CASE("Assert fails in narrow cast") {
    SUBCASE("checked_cast - Positive to larger positive") {
        CHECK_THROWS_AS(checked_cast<uint16_t>(100000), std::runtime_error);
    }

    SUBCASE("checked_cast - Negative to larger negative") {
        CHECK_THROWS_AS(checked_cast<int16_t>(-100000), std::runtime_error);
    }

    SUBCASE("checked_cast - Positive to negative") {
        CHECK_THROWS_AS(checked_cast<uint16_t>(-1), std::runtime_error);
    }

    SUBCASE("checked_cast - Double to int") {
        uint64_t value = (1U << 24U) + 1;  // greater than 2^24, where float precision is 2
        CHECK_THROWS_AS(checked_cast<float>(value), std::runtime_error);
    }

    SUBCASE("checked_cast - Double to int") {
        uint64_t value = (1ULL << 53U) + 1;  // greater than 2^53  where double precision is 2
        CHECK_THROWS_AS(checked_cast<double>(value), std::runtime_error);
    }
}

#endif


TEST_CASE("test_clamp") {
    SUBCASE("Value within range") {
        CHECK(clamp(5, 0, 10) == 5);
        CHECK(clamp(3.14, 0.0, 5.0) == 3.14);
        CHECK(clamp('c', 'a', 'z') == 'c');
    }

    SUBCASE("Value below lower bound") {
        CHECK(clamp(-5, 0, 10) == 0);
        CHECK(clamp(-3.14, 0.0, 5.0) == 0.0);
        CHECK(clamp('a', 'c', 'z') == 'c');
    }

    SUBCASE("Value above upper bound") {
        CHECK(clamp(15, 0, 10) == 10);
        CHECK(clamp(7.5, 0.0, 5.0) == 5.0);
        CHECK(clamp('z', 'a', 'c') == 'c');
    }
}

TEST_CASE("test_abs") {
    SUBCASE("Positive value") {
        CHECK(abs(5) == 5);
        CHECK(abs(3.14) == 3.14);
    }

    SUBCASE("Negative value") {
        CHECK(abs(-5) == 5);
        CHECK(abs(-3.14) == 3.14);
    }

    SUBCASE("Zero value") {
        CHECK(abs(0) == 0);
    }
}

TEST_CASE("test_max") {
    SUBCASE("Integers") {
        CHECK(max(1, 2) == 2);
        CHECK(max(1, 2, 3) == 3);
        CHECK(max(3, 2, 1) == 3);
    }

    SUBCASE("Floating-point numbers") {
        CHECK(max(1.0, 2.0) == 2.0);
        CHECK(max(1.0, 2.0, 3.0) == 3.0);
        CHECK(max(3.0, 2.0, 1.0) == 3.0);
    }
}

TEST_CASE("test_min") {
    SUBCASE("Integers") {
        CHECK(min(1, 2) == 1);
        CHECK(min(1, 2, 3) == 1);
        CHECK(min(3, 2, 1) == 1);
    }

    SUBCASE("Floating-point numbers") {
        CHECK(min(1.0, 2.0) == 1.0);
        CHECK(min(1.0, 2.0, 3.0) == 1.0);
        CHECK(min(3.0, 2.0, 1.0) == 1.0);
    }
}

TEST_CASE("test_any") {
    SUBCASE("Empty container") {
        std::vector<int> empty_container;
        CHECK_FALSE(any(empty_container, [](int x) { return x > 0; }));
    }

    SUBCASE("Container with positive values") {
        int container[] = {1, 2, 3, 4, 5};
        CHECK(any(container, [](int x) { return x > 0; }));
    }

    SUBCASE("Container with negative values") {
        std::array<int, 5> container = {-1, -2, -3, -4, -5};
        CHECK_FALSE(any(container, [](int x) { return x > 0; }));
    }

    SUBCASE("Container with mixed values") {
        int container[5] = {-1, 0, 1, 2, 3};
        CHECK(any(container, [](int x) { return x > 0; }));
    }
}

TEST_CASE("test_all") {
    SUBCASE("Empty container") {
        std::vector<int> empty_container;
        CHECK(all(empty_container, [](int x) { return x > 0; }));
    }

    SUBCASE("Container with positive values") {
        int container[] = {1, 2, 3, 4, 5};
        CHECK(all(container, [](int x) { return x > 0; }));
    }

    SUBCASE("Container with negative values") {
        std::array<int, 5> container = {-1, -2, -3, -4, -5};
        CHECK_FALSE(all(container, [](int x) { return x > 0; }));
    }

    SUBCASE("Container with mixed values") {
        int container[5] = {-1, 0, 1, 2, 3};
        CHECK_FALSE(all(container, [](int x) { return x > 0; }));
    }
}

TEST_CASE("test_size") {
    SUBCASE("Empty container") {
        std::vector<int> empty_container;
        CHECK(cft::size(empty_container) == 0);
    }

    SUBCASE("Non-empty container") {
        std::array<int, 5> container = {1, 2, 3, 4, 5};
        CHECK(cft::size(container) == 5);
    }

    SUBCASE("Array") {
        int arr[] = {1, 2, 3, 4, 5};
        CHECK(size(arr) == 5);
    }
}

TEST_CASE("test_range_min") {
    SUBCASE("Container with positive values") {
        auto container = std::vector<int>{1, 2, 3, 4, 5};
        CHECK(range_min(container, [](int x) { return x; }) == 1);
    }

    SUBCASE("Container with negative values") {
        std::array<int, 5> container = {-1, -2, -3, -4, -5};
        CHECK(range_min(container, [](int x) { return x; }) == -5);
    }

    SUBCASE("Container with mixed values") {
        auto container = std::vector<int>{-1, 0, 1, 2, 3};
        CHECK(range_min(container, [](int x) { return x; }) == -1);
    }
}

TEST_CASE("test_remove_if") {
    SUBCASE("Empty container") {
        std::vector<int> empty_container;
        remove_if(empty_container, [](int x) { return x > 0; });
        CHECK(empty_container.empty());
    }

    SUBCASE("Container with positive values") {
        std::vector<int> container = {1, 2, 3, 4, 5};
        remove_if(container, [](int x) { return x > 3; });
        CHECK(container == std::vector<int>{1, 2, 3});
    }

    SUBCASE("Container with negative values") {
        std::vector<int> container = {-1, -2, -3, -4, -5};
        remove_if(container, [](int x) { return x < -3; });
        CHECK(container == std::vector<int>{-1, -2, -3});
    }

    SUBCASE("Container with mixed values") {
        std::vector<int> container = {-1, 0, 1, 2, 3};
        remove_if(container, [](int x) { return x < 0; });
        CHECK(container == std::vector<int>{0, 1, 2, 3});
    }
}


}  // namespace cft