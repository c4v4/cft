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
}  // namespace cft