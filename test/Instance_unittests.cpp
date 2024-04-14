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
#include <stdexcept>

#include "core/Instance.hpp"

namespace cft {

namespace local { namespace {
    Instance make_partial_inst() {
        ridx_t nrows = 40;

        auto cols = SparseBinMat<ridx_t>();
        cols.push_back({1, 2, 3, 4, 5, 6, 7, 8, 9, 10});
        cols.push_back({11, 12, 13, 14, 15, 16, 17, 18, 19, 20});
        cols.push_back({21, 22, 23, 24, 25, 26, 27, 28, 29, 30});
        cols.push_back({31, 32, 33, 34, 35, 36, 37, 38, 39, 0});
        cols.push_back({1, 11, 21, 31, 2, 12, 22, 32});
        cols.push_back({3, 13, 23, 33, 4, 14, 24, 34});
        cols.push_back({5, 15, 25, 35, 6, 16, 26, 36});

        auto costs    = std::vector<real_t>{1, 2, 3, 4, 5, 6, 7};

        auto inst     = Instance();
        inst.cols     = std::move(cols);
        inst.costs    = std::move(costs);
        inst.rows.resize(nrows);

        return inst;
    }
}  // namespace
}  // namespace local

#ifndef NDEBUG

TEST_CASE("Test fill_rows_from_cols") {
    auto inst = local::make_partial_inst();
    REQUIRE_NOTHROW(fill_rows_from_cols(inst.cols, inst.rows.size(), inst.rows));
    REQUIRE_NOTHROW(col_and_rows_check(inst.cols, inst.rows));
}

TEST_CASE("Test fill_rows_from_cols fail") {
    auto inst = local::make_partial_inst();
    REQUIRE_NOTHROW(fill_rows_from_cols(inst.cols, inst.rows.size(), inst.rows));
    inst.rows.back().push_back(0);
    REQUIRE_THROWS_AS(col_and_rows_check(inst.cols, inst.rows), std::runtime_error);
}
#endif

TEST_CASE("Test push_back_col_from") {
    auto inst1 = local::make_partial_inst();
    auto inst2 = local::make_partial_inst();
    REQUIRE_NOTHROW(push_back_col_from(inst1, 0, inst2));
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