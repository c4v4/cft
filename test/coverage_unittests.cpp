// SPDX-FileCopyrightText: 2024 Francesco Cavaliere <francescocava95@gmail.com>
// SPDX-License-Identifier: MIT

#define CATCH_CONFIG_MAIN
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

    ridx_t nrows = 99_R;
    cidx_t ncols = 6_C;

    auto cols = SparseBinMat<ridx_t>();
    cols.push_back({81_R, 97_R, 12_R, 84_R, 88_R, 50_R, 62_R, 46_R, 25_R, 4_R, 40_R, 16_R, 98_R});
    cols.push_back({70_R, 95_R, 53_R, 86_R, 71_R, 10_R, 26_R, 33_R, 39_R, 61_R, 54_R, 92_R, 31_R});
    cols.push_back({66_R, 32_R, 35_R, 22_R, 78_R, 29_R, 20_R, 11_R, 48_R, 1_R, 42_R, 75_R, 41_R});
    cols.push_back({19_R, 87_R, 9_R, 60_R, 93_R, 83_R, 15_R, 79_R, 37_R, 51_R, 49_R, 3_R, 21_R});
    cols.push_back({13_R, 57_R, 14_R, 82_R, 73_R, 64_R, 85_R, 47_R, 6_R, 96_R, 45_R, 65_R, 74_R});
    cols.push_back({5_R, 91_R, 34_R, 55_R, 0_R, 23_R, 36_R, 56_R, 80_R, 68_R, 67_R, 59_R, 94_R});
    cols.push_back({63_R, 90_R, 24_R, 28_R, 30_R, 72_R, 38_R, 44_R, 2_R, 7_R, 77_R});
    cols.push_back({69_R, 58_R, 18_R, 27_R, 17_R, 43_R, 8_R, 76_R, 52_R, 89_R});


    auto cs = CoverCounters(nrows);
    REQUIRE(cs.cover(cols.idxs) == static_cast<size_t>(nrows));

    cs.reset(nrows);
    for (ridx_t i = 0_R; i < nrows; ++i)
        REQUIRE(cs[i] == 0);
    for (cidx_t j = 0_C; j < ncols; ++j)
        REQUIRE(cs.cover(cols[j]) == cols[j].size());
}

TEST_CASE("test_multiples_rows_cover_set_coverage") {

    ridx_t nrows = 40_R;
    cidx_t ncols = 7_C;

    auto cols = SparseBinMat<ridx_t>();
    cols.push_back({1_R, 2_R, 3_R, 4_R, 5_R, 6_R, 7_R, 8_R, 9_R, 10_R});
    cols.push_back({11_R, 12_R, 13_R, 14_R, 15_R, 16_R, 17_R, 18_R, 19_R, 20_R});
    cols.push_back({21_R, 22_R, 23_R, 24_R, 25_R, 26_R, 27_R, 28_R, 29_R, 30_R});
    cols.push_back({31_R, 32_R, 33_R, 34_R, 35_R, 36_R, 37_R, 38_R, 39_R, 0_R});
    cols.push_back({1_R, 11_R, 21_R, 31_R, 2_R, 12_R, 22_R, 32_R});
    cols.push_back({3_R, 13_R, 23_R, 33_R, 4_R, 14_R, 24_R, 34_R});
    cols.push_back({5_R, 15_R, 25_R, 35_R, 6_R, 16_R, 26_R, 36_R});

    auto cs = CoverCounters(nrows);
    REQUIRE(cs.cover(cols.idxs) == static_cast<size_t>(nrows));

    cs.reset(nrows);
    for (ridx_t i = 0_R; i < rsize(cs); ++i)
        REQUIRE(cs[i] == 0);
    for (cidx_t j = 0_C; j < 4_C; ++j)
        REQUIRE(cs.cover(cols[j]) == cols[j].size());
    for (cidx_t j = 4_C; j < csize(cols); ++j) {
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
    for (ridx_t i = 0_R; i < rsize(cs); ++i)
        cover_count += cs[i];
    for (cidx_t j = 0_C; j < ncols; ++j)
        nnz += cols[j].size();
    REQUIRE(cover_count == nnz);
}

#ifndef NDEBUG

TEST_CASE("Test coverage assert fails") {
    ridx_t nrows = 40_R;

    auto cols = SparseBinMat<int>();
    cols.push_back({40, 12, 13, 14, 15, 40, 17, 18, 19, 20});   // 40!
    cols.push_back({131, 32, 33, 34, 35, 100, 37, 38, 39, 0});  // 131!
    cols.push_back({-1, -11, -21, -31, -2, -12, -22, -32});     // all negative
    cols.push_back({-5, 15, 25, 35, 6, 1, 26, 36});             // -5!

    auto cs = CoverCounters(nrows);
    for (cidx_t j = 0_C; j < 4; ++j) {
        REQUIRE_THROWS_AS(cs.cover(cols[j]), std::runtime_error);
        REQUIRE_THROWS_AS(cs.is_redundant_cover(cols[j]), std::runtime_error);
    }

    for (cidx_t j = 0_C; j < 4; ++j) {
        REQUIRE_THROWS_AS(cs.uncover(cols[j]), std::runtime_error);
        REQUIRE_THROWS_AS(cs.is_redundant_uncover(cols[j]), std::runtime_error);
    }
    REQUIRE_THROWS_AS(cs.uncover(std::vector<int>{0, 0, 0, 0, 0}), std::runtime_error);

    REQUIRE_THROWS_AS(cs[40], std::runtime_error);
    REQUIRE_THROWS_AS(cs[-1], std::runtime_error);
}

#endif

}  // namespace cft