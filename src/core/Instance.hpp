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

#ifndef CFT_SRC_INSTANCE_INSTANCE_HPP
#define CFT_SRC_INSTANCE_INSTANCE_HPP


#include <vector>

#include "core/cft.hpp"
#include "utils/SparseBinMat.hpp"
#include "utils/assert.hpp"  // IWYU pragma:  keep

#ifndef NDEBUG
#include "utils/coverage.hpp"
#include "utils/utility.hpp"
#endif

namespace cft {

// A data structure representing an instance using sparse binary matrix representation.
struct Instance {
    SparseBinMat<ridx_t>             cols;
    std::vector<std::vector<cidx_t>> rows;
    std::vector<real_t>              costs;
};

#ifndef NDEBUG
inline void col_and_rows_check(SparseBinMat<ridx_t> const&             cols,
                               std::vector<std::vector<cidx_t>> const& rows) {
    for (cidx_t j = 0_C; j < csize(cols); ++j) {
        assert("Col is empty" && !cols[j].empty());
        assert("Col does not exist" && j < csize(cols));
        for (ridx_t i : cols[j])
            assert("Col not in row" && any(rows[i], [j](cidx_t rj) { return rj == j; }));
    }

    for (ridx_t i = 0_R; i < rsize(rows); ++i) {
        assert("Row is empty" && !rows[i].empty());
        assert("Row does not exist" && i < rsize(rows));
        for (cidx_t j : rows[i])
            assert("Row not in col" && any(cols[j], [i](ridx_t ci) { return ci == i; }));
    }
}

// TODO(any): find a better place for this function.
inline void check_solution(Instance const& inst, Solution const& sol) {
    ridx_t nrows = rsize(inst.rows);

    // check coverage
    ridx_t covered_rows = 0_R;
    auto   row_coverage = CoverCounters<>(nrows);
    for (auto j : sol.idxs)
        covered_rows += as_ridx(row_coverage.cover(inst.cols[j]));
    assert(covered_rows == nrows);

    // check cost
    real_t total_cost = 0.0_F;
    for (cidx_t j : sol.idxs)
        total_cost += inst.costs[j];
    assert(-1e-6_F < total_cost - sol.cost && total_cost - sol.cost < 1e-6_F);
}
#endif

// Completes instance initialization by creating rows
inline void fill_rows_from_cols(SparseBinMat<ridx_t> const&       cols,
                                ridx_t                            nrows,
                                std::vector<std::vector<cidx_t>>& rows) {
    rows.resize(nrows);
    for (auto& row : rows) {
        row.clear();
        row.reserve(csize(cols.idxs) / nrows);
    }

    for (cidx_t j = 0_C; j < csize(cols); ++j)
        for (ridx_t i : cols[j])
            rows[i].push_back(j);
    // CFT_IF_DEBUG(col_and_rows_check(cols, rows));
}

// Copy a column from one instance to another pushing it back as last column.
// The main utility of this function is to avoid forgetting somethign in the copy.
inline void push_back_col_from(Instance const& src_inst, cidx_t j, Instance& dest_inst) {
    dest_inst.cols.push_back(src_inst.cols[j]);
    dest_inst.costs.push_back(src_inst.costs[j]);
}

// Clear all data of an instance creating an empty instance inplace.
inline void clear_inst(Instance& inst) {
    inst.cols.clear();
    inst.rows.clear();
    inst.costs.clear();
}

// For core instance we only need column mappings, since the rows remain the same.
struct InstAndMap {
    Instance            inst;
    std::vector<cidx_t> col_map;
};

// Generic mappings between instances of columns and rows indexes.
struct IdxsMaps {
    std::vector<cidx_t> col_map;
    std::vector<ridx_t> row_map;
};
}  // namespace cft


#endif /* CFT_SRC_INSTANCE_INSTANCE_HPP */
