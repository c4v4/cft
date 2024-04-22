// SPDX-FileCopyrightText: 2024 Francesco Cavaliere <francescocava95@gmail.com>
// SPDX-License-Identifier: MIT

#include <array>
#include <catch2/catch.hpp>

#ifndef NDEBUG
#include <stdexcept>
#endif

#include "utils/utility.hpp"

namespace cft {

TEST_CASE("checked_cast - Test with integers") {
    SECTION("checked_cast - Positive to smaller positive") {
        auto result = checked_cast<int16_t>(42);
        REQUIRE(result == 42);
    }

    SECTION("checked_cast - Negative to smaller negative") {
        auto result = checked_cast<int16_t>(-42);
        REQUIRE(result == -42);
    }

    SECTION("checked_cast - Float to int") {
        float value = 3.14F;
        REQUIRE(checked_cast<int16_t>(value) == 3);
    }

    SECTION("checked_cast - Double to int") {
        double value = 1212.14;
        REQUIRE(checked_cast<int16_t>(value) == 1212);
    }
}

#ifndef NDEBUG

TEST_CASE("Assert fails in narrow cast") {
    SECTION("checked_cast - Positive to larger positive") {
        REQUIRE_THROWS_AS(checked_cast<uint16_t>(100000), std::runtime_error);
    }

    SECTION("checked_cast - Negative to larger negative") {
        REQUIRE_THROWS_AS(checked_cast<int16_t>(-100000), std::runtime_error);
    }

    SECTION("checked_cast - Positive to negative") {
        REQUIRE_THROWS_AS(checked_cast<uint16_t>(-1), std::runtime_error);
    }

    SECTION("checked_cast - Double to int") {
        uint64_t value = (1U << 24U) + 1;  // greater than 2^24, where float precision is 2
        REQUIRE_THROWS_AS(checked_cast<float>(value), std::runtime_error);
    }

    SECTION("checked_cast - Double to int") {
        uint64_t value = (1ULL << 53U) + 1;  // greater than 2^53  where double precision is 2
        REQUIRE_THROWS_AS(checked_cast<double>(value), std::runtime_error);
    }
}

#endif


TEST_CASE("test_clamp") {
    SECTION("Value within range") {
        REQUIRE(clamp(5, 0, 10) == 5);
        REQUIRE(clamp(3.14, 0.0, 5.0) == 3.14);
        REQUIRE(clamp('c', 'a', 'z') == 'c');
    }

    SECTION("Value below lower bound") {
        REQUIRE(clamp(-5, 0, 10) == 0);
        REQUIRE(clamp(-3.14, 0.0, 5.0) == 0.0);
        REQUIRE(clamp('a', 'c', 'z') == 'c');
    }

    SECTION("Value above upper bound") {
        REQUIRE(clamp(15, 0, 10) == 10);
        REQUIRE(clamp(7.5, 0.0, 5.0) == 5.0);
        REQUIRE(clamp('z', 'a', 'c') == 'c');
    }
}

TEST_CASE("test_abs") {
    SECTION("Positive value") {
        REQUIRE(abs(5) == 5);
        REQUIRE(abs(3.14) == 3.14);
    }

    SECTION("Negative value") {
        REQUIRE(abs(-5) == 5);
        REQUIRE(abs(-3.14) == 3.14);
    }

    SECTION("Zero value") {
        REQUIRE(abs(0) == 0);
    }
}

TEST_CASE("test_max") {
    SECTION("Integers") {
        REQUIRE(max(1) == 1);
        REQUIRE(max(1, 2) == 2);
        REQUIRE(max(1, 2, 3) == 3);
        REQUIRE(max(3, 2, 1) == 3);
    }

    SECTION("Floating-point numbers") {
        REQUIRE(max(1.0) == 1.0);
        REQUIRE(max(1.0, 2.0) == 2.0);
        REQUIRE(max(1.0, 2.0, 3.0) == 3.0);
        REQUIRE(max(3.0, 2.0, 1.0) == 3.0);
    }

    SECTION("Booleans") {
        REQUIRE(max(true) == true);
        REQUIRE(max(true, false) == true);
        REQUIRE(max(true, false, true) == true);
        REQUIRE(max(false, true, false) == true);
    }
}

TEST_CASE("test_min") {
    SECTION("Integers") {
        REQUIRE(min(1) == 1);
        REQUIRE(min(1, 2) == 1);
        REQUIRE(min(1, 2, 3) == 1);
        REQUIRE(min(3, 2, 1) == 1);
    }

    SECTION("Floating-point numbers") {
        REQUIRE(min(1.0) == 1.0);
        REQUIRE(min(1.0, 2.0) == 1.0);
        REQUIRE(min(1.0, 2.0, 3.0) == 1.0);
        REQUIRE(min(3.0, 2.0, 1.0) == 1.0);
    }

    SECTION("Booleans") {
        REQUIRE(min(true) == true);
        REQUIRE(min(true, false) == false);
        REQUIRE(min(true, false, true) == false);
        REQUIRE(min(false, true, false) == false);
    }
}

TEST_CASE("test_any") {
    SECTION("Empty container") {
        std::vector<int> empty_container;
        REQUIRE_FALSE(any(empty_container, [](int x) { return x > 0; }));
    }

    SECTION("Container with positive values") {
        int container[] = {1, 2, 3, 4, 5};
        REQUIRE(any(container, [](int x) { return x > 0; }));
    }

    SECTION("Container with negative values") {
        std::array<int, 5> container = {-1, -2, -3, -4, -5};
        REQUIRE_FALSE(any(container, [](int x) { return x > 0; }));
    }

    SECTION("Container with mixed values") {
        int container[5] = {-1, 0, 1, 2, 3};
        REQUIRE(any(container, [](int x) { return x > 0; }));
    }
}

TEST_CASE("test_all") {
    SECTION("Empty container") {
        std::vector<int> empty_container;
        REQUIRE(all(empty_container, [](int x) { return x > 0; }));
    }

    SECTION("Container with positive values") {
        int container[] = {1, 2, 3, 4, 5};
        REQUIRE(all(container, [](int x) { return x > 0; }));
    }

    SECTION("Container with negative values") {
        std::array<int, 5> container = {-1, -2, -3, -4, -5};
        REQUIRE_FALSE(all(container, [](int x) { return x > 0; }));
    }

    SECTION("Container with mixed values") {
        int container[5] = {-1, 0, 1, 2, 3};
        REQUIRE_FALSE(all(container, [](int x) { return x > 0; }));
    }
}

TEST_CASE("test_size") {
    SECTION("Empty container") {
        std::vector<int> empty_container;
        REQUIRE(cft::size(empty_container) == 0);
    }

    SECTION("Non-empty container") {
        std::array<int, 5> container = {1, 2, 3, 4, 5};
        REQUIRE(cft::size(container) == 5);
    }

    SECTION("Array") {
        int arr[] = {1, 2, 3, 4, 5};
        REQUIRE(size(arr) == 5);
    }
}

TEST_CASE("test_range_min") {
    SECTION("Container with positive values") {
        auto container = std::vector<int>{1, 2, 3, 4, 5};
        REQUIRE(range_min(container, [](int x) { return x; }) == 1);
    }

    SECTION("Container with negative values") {
        std::array<int, 5> container = {-1, -2, -3, -4, -5};
        REQUIRE(range_min(container, [](int x) { return x; }) == -5);
    }

    SECTION("Container with mixed values") {
        auto container = std::vector<int>{-1, 0, 1, 2, 3};
        REQUIRE(range_min(container, [](int x) { return x; }) == -1);
    }
}

TEST_CASE("test_remove_if") {
    SECTION("Empty container") {
        std::vector<int> empty_container;
        remove_if(empty_container, [](int x) { return x > 0; });
        REQUIRE(empty_container.empty());
    }

    SECTION("Container with positive values") {
        std::vector<int> container = {1, 2, 3, 4, 5};
        remove_if(container, [](int x) { return x > 3; });
        REQUIRE(container == std::vector<int>{1, 2, 3});
    }

    SECTION("Container with negative values") {
        std::vector<int> container = {-1, -2, -3, -4, -5};
        remove_if(container, [](int x) { return x < -3; });
        REQUIRE(container == std::vector<int>{-1, -2, -3});
    }

    SECTION("Container with mixed values") {
        std::vector<int> container = {-1, 0, 1, 2, 3};
        remove_if(container, [](int x) { return x < 0; });
        REQUIRE(container == std::vector<int>{0, 1, 2, 3});
    }
}


}  // namespace cft