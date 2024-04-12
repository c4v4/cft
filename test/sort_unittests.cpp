#include <catch2/catch.hpp>
#include <vector>

#include "../src/utils/sort.hpp"

TEST_CASE("sort function sorts the container in ascending order", "[sort]") {
    SECTION("Sort an empty container") {
        std::vector<int> container;
        cft::sort(container);
        REQUIRE(container.empty());
    }

    SECTION("Sort a container with one element") {
        std::vector<int> container = {5};
        cft::sort(container);
        REQUIRE(container == std::vector<int>{5});
    }

    SECTION("Sort a container with multiple elements") {
        std::vector<int> container = {5, 2, 8, 1, 9};
        cft::sort(container);
        REQUIRE(container == std::vector<int>{1, 2, 5, 8, 9});
    }
}

TEST_CASE("nth_element function moves the nth element to its correct position", "[nth_element]") {
    SECTION("nth_element on an empty container") {
        std::vector<int> container;
        cft::nth_element(container, 0);
        REQUIRE(container.empty());
    }

    SECTION("nth_element on a container with one element") {
        std::vector<int> container = {5};
        cft::nth_element(container, 0);
        REQUIRE(container == std::vector<int>{5});
    }

    SECTION("nth_element on a container with multiple elements") {
        std::vector<int> container = {5, 2, 8, 1, 9};
        cft::nth_element(container, 2);
        REQUIRE(container[2] == 5);
    }

    SECTION("nth_element on a container with duplicate elements") {
        std::vector<int> container = {1, 2, 1, 1, 2};
        cft::nth_element(container, 2);
        REQUIRE(container[2] == 1);
    }
}
