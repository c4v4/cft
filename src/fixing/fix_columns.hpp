// Copyright (c) 2024 Francesco Cavaliere
// This program is free software: you can redistribute it and/or modify it under the terms of the
// GNU General Public License as published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version. This program is distributed in the hope that it
// will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details. You should
// have received a copy of the GNU General Public License along with this program. If not, see
// <https://www.gnu.org/licenses/>.

#ifndef CFT_SRC_FIXING_FIX_COLUMNS_HPP
#define CFT_SRC_FIXING_FIX_COLUMNS_HPP

#include <vector>

#include "core/cft.hpp"
#ifndef NDEBUG
#include <algorithm>
#endif

#include "instance/Instance.hpp"

namespace cft {

struct FixingData {
    std::vector<cidx_t> old2new_col_map;
    std::vector<ridx_t> old2new_row_map;
    std::vector<cidx_t> new2old_col_map;
    std::vector<ridx_t> new2old_row_map;
    std::vector<cidx_t> fixed_cols;
    real_t              fixed_cost;
};

inline FixingData make_identity_fixing_data(cidx_t ncols, ridx_t nrows) {
    auto fixing            = FixingData();
    fixing.old2new_col_map = std::vector<cidx_t>(ncols);
    fixing.old2new_row_map = std::vector<ridx_t>(nrows);
    fixing.new2old_col_map = std::vector<cidx_t>(ncols);
    fixing.new2old_row_map = std::vector<ridx_t>(nrows);
    fixing.fixed_cols      = std::vector<cidx_t>();
    fixing.fixed_cost      = 0.0;

    for (cidx_t j = 0; j < ncols; ++j) {
        fixing.old2new_col_map[j] = j;
        fixing.new2old_col_map[j] = j;
    }
    for (ridx_t i = 0; i < nrows; ++i) {
        fixing.old2new_row_map[i] = i;
        fixing.new2old_row_map[i] = i;
    }
    return fixing;
}

namespace {
#ifndef NDEBUG
    void mappings_check(Instance const& old_inst, Instance const& inst, FixingData& fixing) {
        for (cidx_t old_j = 0; old_j < old_inst.cols.size(); ++old_j) {

            cidx_t new_j = fixing.old2new_col_map[old_j];
            if (any(fixing.fixed_cols, [&](cidx_t j) { return j == old_j; }))
                assert(new_j == CFT_REMOVED_IDX);
            if (new_j == CFT_REMOVED_IDX)
                continue;

            assert(!inst.cols[new_j].empty());
            assert(inst.cols[new_j].size() <= old_inst.cols[old_j].size());
            for (ridx_t r = 0; r < inst.cols[new_j].size(); ++r) {
                ridx_t old_i = old_inst.cols[old_j][r];
                ridx_t new_i = fixing.old2new_row_map[old_i];
                if (new_i == CFT_REMOVED_IDX) {
                    any(old_inst.rows[old_i], [](cidx_t j) { return j == CFT_REMOVED_IDX; });
                    continue;
                }
                assert(std::count(inst.cols[new_j].begin(), inst.cols[new_j].end(), new_i) == 1);
                assert(std::count(inst.rows[new_i].begin(), inst.rows[new_i].end(), new_j) == 1);
                assert(inst.rows[new_i].size() <= old_inst.rows[old_i].size());
                assert(!inst.rows[new_i].empty());
            }
        }
    }
#endif

    // Mark columns and rows to be removed and update fixed cols and costs
    ridx_t mark_and_update_fixed_elements(Instance&                  inst,
                                          std::vector<cidx_t> const& cols_to_fix,
                                          FixingData&                fixing) {
        size_t removed_rows = 0;
        for (cidx_t j : cols_to_fix) {
            cidx_t& orig_j = fixing.old2new_col_map[j];
            assert("Columns removed twice" && orig_j != CFT_REMOVED_IDX);

            fixing.fixed_cost += inst.costs[j];   // update fixed cost with new fixing
            fixing.fixed_cols.push_back(orig_j);  // add new fixed indexes
            orig_j = CFT_REMOVED_IDX;             // mark column to be removed
            for (ridx_t i : inst.cols[j]) {
                removed_rows += fixing.old2new_row_map[i] != CFT_REMOVED_IDX ? 1 : 0;
                fixing.old2new_row_map[i] = CFT_REMOVED_IDX;  // mark row to be removed
            }
        }
        return removed_rows;
    }

    void set_inst_as_empty(Instance& inst, FixingData& fixing) {
        inst.cols.clear();
        inst.rows.clear();
        inst.costs.clear();
        inst.solcosts.clear();

        fixing.old2new_col_map.clear();
        fixing.old2new_row_map.clear();
        fixing.new2old_col_map.clear();
        fixing.new2old_row_map.clear();
    }

    // Remove marked rows and make old->new row mapping
    void adjust_rows_pos_and_fill_map(Instance& inst, FixingData& fixing) {
        ridx_t old_nrows = inst.rows.size();
        fixing.old2new_row_map.assign(old_nrows, CFT_REMOVED_IDX);
        ridx_t new_i = 0;
        for (ridx_t old_i = 0; old_i < old_nrows; ++old_i) {
            ridx_t orig_i = fixing.new2old_row_map[old_i];
            if (orig_i != CFT_REMOVED_IDX) {
                assert(!inst.rows[old_i].empty());
                if (new_i != old_i) {
                    inst.rows[new_i]              = std::move(inst.rows[old_i]);
                    fixing.new2old_row_map[new_i] = orig_i;
                }
                ++new_i;
            }
        }
        fixing.new2old_row_map.resize(new_i);
        inst.rows.resize(new_i);
    }

    // Remove marked columns adjusting row indexes and make old->new col mapping
    void adjust_cols_pos_and_idxs_and_fill_map(Instance& inst, FixingData& fixing) {
        cidx_t old_ncols = inst.cols.size();
        fixing.old2new_col_map.assign(old_ncols, CFT_REMOVED_IDX);
        cidx_t new_j = 0;
        size_t n     = 0;
        for (ridx_t old_j = 0; old_j < old_ncols; ++old_j) {

            cidx_t orig_j = fixing.new2old_col_map[old_j];
            if (orig_j == CFT_REMOVED_IDX)
                continue;

            size_t nbeg = n;  // save here to set cols.begs[new_j] later
            for (size_t o = inst.cols.begs[old_j]; o < inst.cols.begs[old_j + 1]; ++o) {
                ridx_t old_i = inst.cols.idxs[o];
                ridx_t new_i = fixing.old2new_row_map[old_i];
                if (new_i != CFT_REMOVED_IDX)
                    inst.cols.idxs[n++] = new_i;
            }
            if (n == nbeg)
                continue;

            inst.cols.begs[new_j]         = nbeg;  // here to not invalidate o begin
            inst.costs[new_j]             = inst.costs[old_j];
            inst.solcosts[new_j]          = inst.solcosts[old_j];
            fixing.new2old_col_map[new_j] = orig_j;
            fixing.old2new_col_map[old_j] = new_j;
            ++new_j;
        }
        inst.cols.begs[new_j] = n;
        inst.cols.idxs.resize(n);
        inst.cols.begs.resize(new_j + 1);
        inst.costs.resize(new_j);
        inst.solcosts.resize(new_j);
        fixing.new2old_col_map.resize(new_j);
    }

    // Adjust column indexes stored in eanch row
    void adjust_rows_idxs(Instance& inst, FixingData& fixing) {
        for (auto& row : inst.rows) {
            cidx_t w = 0;
            for (cidx_t r = 0; r < row.size(); ++r) {
                cidx_t new_j = fixing.old2new_col_map[row[r]];
                if (new_j != CFT_REMOVED_IDX)
                    row[w++] = new_j;
            }
            assert(w > 0);
            row.resize(w);
        }
    }
}  // namespace

// Modifies instance by fixing columns in-place. New indexes are always <= old ones, allowing
// in-place external data structure updates. Note: Column fixing is irreversible, i.e., you cannot
// get the original instance from the subinstance.
inline void fix_columns(Instance&                  inst,
                        std::vector<cidx_t> const& cols_to_fix,
                        FixingData&                fixing) {

    fixing.old2new_col_map.clear();
    fixing.old2new_row_map.clear();
    if (cols_to_fix.empty())
        return;

    IF_DEBUG(auto old_inst = inst);

    ridx_t removed_rows = mark_and_update_fixed_elements(inst, cols_to_fix, fixing);
    if (removed_rows == inst.rows.size())  // If all rows were removed, clear everything
        return set_inst_as_empty(inst, fixing);

    // Map old rows and columns to new ones based on whats has been marked
    adjust_rows_pos_and_fill_map(inst, fixing);
    adjust_cols_pos_and_idxs_and_fill_map(inst, fixing);
    adjust_rows_idxs(inst, fixing);

    IF_DEBUG(col_and_rows_check(inst.cols, inst.rows));  // coherent instance
    IF_DEBUG(mappings_check(old_inst, inst, fixing));

    // TODO(cava): reductions step (e.g., 1-col rows)
}

}  // namespace cft

#endif /* CFT_SRC_FIXING_FIX_COLUMNS_HPP */
