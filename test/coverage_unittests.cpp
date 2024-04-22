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
#include <cstring>

#ifndef NDEBUG
#include <stdexcept>
#endif

#include "core/cft.hpp"
#include "utils/CoverCounters.hpp"
#include "utils/SparseBinMat.hpp"

namespace cft {

TEST_CASE("test_single_row_cover_set_coverage") {

    ridx_t nrows = 99;
    cidx_t ncols = 6;

    auto cols = SparseBinMat<ridx_t>();
    cols.push_back({81, 97, 12, 84, 88, 50, 62, 46, 25, 4, 40, 16, 98, 63});
    cols.push_back({70, 95, 53, 86, 71, 10, 26, 33, 39, 61, 54, 92, 31, 90, 24, 28, 30, 72});
    cols.push_back({66, 32, 35, 22, 78, 29, 20, 11, 48, 1, 42, 75, 41, 38, 44, 2, 7, 77, 69});
    cols.push_back({19, 87, 9, 60, 93, 83, 15, 79, 37, 51, 49, 3, 21, 58});
    cols.push_back({13, 57, 14, 82, 73, 64, 85, 47, 6, 96, 45, 65, 74, 18, 27, 17, 43, 8});
    cols.push_back({5, 91, 34, 55, 0, 23, 36, 56, 80, 68, 67, 59, 94, 76, 52, 89});

    auto cs = CoverCounters(nrows);
    REQUIRE(cs.cover(cols.idxs) == static_cast<size_t>(nrows));

    cs.reset(nrows);
    for (ridx_t i = 0; i < nrows; ++i)
        REQUIRE(cs[i] == 0);
    for (cidx_t j = 0; j < ncols; ++j)
        REQUIRE(cs.cover(cols[j]) == cols[j].size());
}

TEST_CASE("test_multiples_rows_cover_set_coverage") {

    ridx_t nrows = 40;
    cidx_t ncols = 7;

    auto cols = SparseBinMat<ridx_t>();
    cols.push_back({1, 2, 3, 4, 5, 6, 7, 8, 9, 10});
    cols.push_back({11, 12, 13, 14, 15, 16, 17, 18, 19, 20});
    cols.push_back({21, 22, 23, 24, 25, 26, 27, 28, 29, 30});
    cols.push_back({31, 32, 33, 34, 35, 36, 37, 38, 39, 0});
    cols.push_back({1, 11, 21, 31, 2, 12, 22, 32});
    cols.push_back({3, 13, 23, 33, 4, 14, 24, 34});
    cols.push_back({5, 15, 25, 35, 6, 16, 26, 36});

    auto cs = CoverCounters(nrows);
    REQUIRE(cs.cover(cols.idxs) == static_cast<size_t>(nrows));

    cs.reset(nrows);
    for (ridx_t i = 0; i < rsize(cs); ++i)
        REQUIRE(cs[i] == 0);
    for (cidx_t j = 0; j < 4; ++j)
        REQUIRE(cs.cover(cols[j]) == cols[j].size());
    for (cidx_t j = 4; j < csize(cols); ++j) {
        REQUIRE(cs.is_redundant_cover(cols[j]));
        REQUIRE(!cs.is_redundant_uncover(cols[j]));
        REQUIRE(cs.cover(cols[j]) == 0);

        REQUIRE(cs.is_redundant_cover(cols[j]));
        REQUIRE(cs.is_redundant_uncover(cols[j]));
        REQUIRE(cs.uncover(cols[j]) == 0);

        REQUIRE(cs.is_redundant_cover(cols[j]));
        REQUIRE(!cs.is_redundant_uncover(cols[j]));
        REQUIRE(cs.uncover(cols[j]) == cols[j].size());

        REQUIRE(!cs.is_redundant_cover(cols[j]));
    }

    cs.reset(nrows);
    REQUIRE(cs.cover(cols.idxs) == static_cast<size_t>(nrows));

    size_t cover_count = 0;
    size_t nnz         = 0;
    for (ridx_t i = 0; i < rsize(cs); ++i)
        cover_count += cs[i];
    for (cidx_t j = 0; j < ncols; ++j)
        nnz += cols[j].size();
    REQUIRE(cover_count == nnz);
}

#ifndef NDEBUG

TEST_CASE("Test coverage assert fails") {
    ridx_t nrows = 40;

    auto cols = SparseBinMat<int>();
    cols.push_back({40, 12, 13, 14, 15, 40, 17, 18, 19, 20});   // 40!
    cols.push_back({131, 32, 33, 34, 35, 100, 37, 38, 39, 0});  // 131!
    cols.push_back({-1, -11, -21, -31, -2, -12, -22, -32});     // all negative
    cols.push_back({-5, 15, 25, 35, 6, 1, 26, 36});             // -5!

    auto cs = CoverCounters(nrows);
    for (cidx_t j = 0; j < 4; ++j) {
        REQUIRE_THROWS_AS(cs.cover(cols[j]), std::runtime_error);
        REQUIRE_THROWS_AS(cs.is_redundant_cover(cols[j]), std::runtime_error);
    }

    for (cidx_t j = 0; j < 4; ++j) {
        REQUIRE_THROWS_AS(cs.uncover(cols[j]), std::runtime_error);
        REQUIRE_THROWS_AS(cs.is_redundant_uncover(cols[j]), std::runtime_error);
    }
    REQUIRE_THROWS_AS(cs.uncover(std::vector<int>{0, 0, 0, 0, 0}), std::runtime_error);

    REQUIRE_THROWS_AS(cs[40], std::runtime_error);
    REQUIRE_THROWS_AS(cs[-1], std::runtime_error);
}

#endif

}  // namespace cft