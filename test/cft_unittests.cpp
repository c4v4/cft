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

#include "core/cft.hpp"

namespace cft {

TEST_CASE("Test CidxAndCost struct", "[cft]") {
    auto c = CidxAndCost{1, 2.5};

    REQUIRE(c.idx == 1);
    REQUIRE(c.cost == 2.5);
}

TEST_CASE("Test Solution struct", "[cft]") {
    auto s = Solution();
    REQUIRE(s.idxs.empty());
    REQUIRE(s.cost == limits<real_t>::inf());
}

TEST_CASE("Test as_cidx function", "[cft]") {
    int    i = 10;
    cidx_t c = as_cidx(i);

    REQUIRE(c == cidx_t{10});
}

TEST_CASE("Test as_ridx function", "[cft]") {
    int    i = 5;
    ridx_t r = as_ridx(i);

    REQUIRE(r == ridx_t{5});
}

TEST_CASE("Test as_real function", "[cft]") {
    double i = 5.55;
    real_t r = as_real(i);

    REQUIRE(r == real_t{5.55});
}

TEST_CASE("Test user-defined literals", "[cft]") {
    cidx_t c = 5_C;
    ridx_t r = 10_R;
    real_t f = 2.5_F;

    REQUIRE(c == cidx_t{5});
    REQUIRE(r == ridx_t{10});
    REQUIRE(f == real_t{2.5});
}

TEST_CASE("Test csize function", "[cft]") {
    auto   vec  = std::vector<int>{1, 2, 3};
    cidx_t size = csize(vec);

    REQUIRE(size == 3_C);
}

TEST_CASE("Test rsize function", "[cft]") {
    auto   vec  = std::vector<int>{1, 2, 3};
    ridx_t size = rsize(vec);

    REQUIRE(size == 3_R);
}

#ifndef NDEBUG

TEST_CASE("Test as_cidx function failure ", "[cft]") {
    if (static_cast<uint64_t>(limits<cidx_t>::max()) < limits<uint64_t>::max())
        REQUIRE_THROWS_AS(as_cidx(limits<uint64_t>::max()), std::runtime_error);
    if (!std::is_signed<cidx_t>::value)
        REQUIRE_THROWS_AS(as_cidx(-1), std::runtime_error);
}

TEST_CASE("Test as_ridx function failure ", "[cft]") {
    if (static_cast<uint64_t>(limits<ridx_t>::max()) < limits<uint64_t>::max())
        REQUIRE_THROWS_AS(as_ridx(limits<uint64_t>::max()), std::runtime_error);
    if (!std::is_signed<ridx_t>::value)
        REQUIRE_THROWS_AS(as_ridx(-1), std::runtime_error);
}

TEST_CASE("Test as_real function failure ", "[cft]") {
    if (static_cast<long double>(limits<real_t>::max()) < limits<long double>::max())
        REQUIRE_THROWS_AS(1e+4932_F, std::runtime_error);
     if (static_cast<long double>(limits<real_t>::max()) < limits<long double>::max())
        REQUIRE_THROWS_AS(as_real(1e+4932L), std::runtime_error);
}

#endif

}  // namespace cft