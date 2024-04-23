// SPDX-FileCopyrightText: 2024 Francesco Cavaliere <francescocava95@gmail.com>
// SPDX-License-Identifier: MIT

#define CATCH_CONFIG_MAIN 
#include <catch2/catch.hpp>
#include <stdexcept>

#include "core/Instance.hpp"
#include "core/cft.hpp"

namespace cft {

namespace local { namespace {
    Instance make_partial_inst() {
        ridx_t nrows = 40_R;

        auto cols = SparseBinMat<ridx_t>();
        cols.push_back({1_R, 2_R, 3_R, 4_R, 5_R, 6_R, 7_R, 8_R, 9_R, 10_R});
        cols.push_back({11_R, 12_R, 13_R, 14_R, 15_R, 16_R, 17_R, 18_R, 19_R, 20_R});
        cols.push_back({21_R, 22_R, 23_R, 24_R, 25_R, 26_R, 27_R, 28_R, 29_R, 30_R});
        cols.push_back({31_R, 32_R, 33_R, 34_R, 35_R, 36_R, 37_R, 38_R, 39_R, 0_R});
        cols.push_back({1_R, 11_R, 21_R, 31_R, 2_R, 12_R, 22_R, 32_R});
        cols.push_back({3_R, 13_R, 23_R, 33_R, 4_R, 14_R, 24_R, 34_R});
        cols.push_back({5_R, 15_R, 25_R, 35_R, 6_R, 16_R, 26_R, 36_R});

        auto costs = std::vector<real_t>{1.0_F, 2.0_F, 3.0_F, 4.0_F, 5.0_F, 6.0_F, 7.0_F};

        auto inst  = Instance();
        inst.cols  = std::move(cols);
        inst.costs = std::move(costs);
        inst.rows.resize(nrows);

        return inst;
    }
}  // namespace
}  // namespace local

#ifndef NDEBUG

TEST_CASE("Test fill_rows_from_cols") {
    auto inst = local::make_partial_inst();
    REQUIRE_NOTHROW(fill_rows_from_cols(inst.cols, rsize(inst.rows), inst.rows));
    REQUIRE_NOTHROW(col_and_rows_check(inst.cols, inst.rows));
}

TEST_CASE("Test fill_rows_from_cols fail") {
    auto inst = local::make_partial_inst();
    REQUIRE_NOTHROW(fill_rows_from_cols(inst.cols, rsize(inst.rows), inst.rows));
    inst.rows.back().push_back(0_C);
    REQUIRE_THROWS_AS(col_and_rows_check(inst.cols, inst.rows), std::runtime_error);
}
#endif

TEST_CASE("Test push_back_col_from") {
    auto inst1 = local::make_partial_inst();
    auto inst2 = local::make_partial_inst();
    REQUIRE_NOTHROW(push_back_col_from(inst1, 0_C, inst2));
    REQUIRE(inst2.cols.size() == inst1.cols.size() + 1);

    auto last_col = inst2.cols[inst2.cols.size() - 1];
    REQUIRE(last_col.size() == inst1.cols[0].size());
    for (size_t n = 0; n < inst1.cols[0].size(); ++n)
        REQUIRE(last_col[n] == inst1.cols[0][n]);

    REQUIRE(inst2.costs.size() == inst1.costs.size() + 1);
}

TEST_CASE("Test clear inst") {
    auto inst = local::make_partial_inst();
    REQUIRE_NOTHROW(clear_inst(inst));
    REQUIRE(inst.cols.empty());
    REQUIRE(inst.rows.empty());
    REQUIRE(inst.costs.empty());
}


}  // namespace cft