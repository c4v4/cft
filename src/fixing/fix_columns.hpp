// Copyright (c) 2024 Luca Accorsi and Francesco Cavaliere
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
#ifndef NDEBUG
#include <algorithm>
#endif


#include "core/cft.hpp"
#include "instance/Instance.hpp"

namespace cft {

struct IdxsMaps {
    std::vector<cidx_t> col_map;
    std::vector<ridx_t> row_map;
};

struct FixingData {
    IdxsMaps            curr2orig;
    std::vector<cidx_t> fixed_cols;
    real_t              fixed_cost = 0.0;
};

inline void make_identity_fixing_data(cidx_t ncols, ridx_t nrows, FixingData& fixing) {
    fixing.curr2orig.col_map.resize(ncols);
    fixing.curr2orig.row_map.resize(nrows);
    fixing.fixed_cols.clear();
    fixing.fixed_cost = 0.0;

    for (cidx_t j = 0; j < ncols; ++j)
        fixing.curr2orig.col_map[j] = j;
    for (ridx_t i = 0; i < nrows; ++i)
        fixing.curr2orig.row_map[i] = i;
}

namespace {
#ifndef NDEBUG
// inline void mappings_check(Instance const& prev_inst,
//                            Instance const& curr_inst,
//                            IdxsMaps const& prev2curr) {
//     for (cidx_t prev_j = 0; prev_j < prev_inst.cols.size(); ++prev_j) {
//
//        cidx_t curr_j = prev2curr.col_map[prev_j];
//        if (curr_j == CFT_REMOVED_IDX)
//            continue;
//
//        assert(!curr_inst.cols[curr_j].empty());
//        assert(curr_inst.cols[curr_j].size() <= prev_inst.cols[prev_j].size());
//        for (ridx_t r = 0; r < prev_inst.cols[prev_j].size(); ++r) {
//            ridx_t prev_i = prev_inst.cols[prev_j][r];
//            ridx_t curr_i = prev2curr.row_map[prev_i];
//            if (curr_i == CFT_REMOVED_IDX) {
//                assert(any(prev_inst.rows[prev_i],
//                           [&](cidx_t j) { return prev2curr.col_map[j] == CFT_REMOVED_IDX; }));
//                continue;
//            }
//            assert(std::count(curr_inst.cols[curr_j].begin(),
//                              curr_inst.cols[curr_j].end(),
//                              curr_i) == 1);
//            assert(std::count(curr_inst.rows[curr_i].begin(),
//                              curr_inst.rows[curr_i].end(),
//                              curr_j) == 1);
//            assert(curr_inst.rows[curr_i].size() <= prev_inst.rows[prev_i].size());
//            assert(!curr_inst.rows[curr_i].empty());
//        }
//    }
//}
#endif

    // Mark columns and rows to be removed and update fixed cols and costs
    inline ridx_t mark_and_update_fixed_elements(Instance&                  inst,
                                                 std::vector<cidx_t> const& cols_to_fix,
                                                 FixingData&                fixing) {
        size_t removed_rows = 0;
        for (cidx_t prev_j : cols_to_fix) {
            cidx_t& orig_j = fixing.curr2orig.col_map[prev_j];
            assert("Columns removed twice" && orig_j != CFT_REMOVED_IDX);

            fixing.fixed_cost += inst.costs[prev_j];  // update fixed cost with new fixing
            fixing.fixed_cols.push_back(orig_j);      // add new fixed indexes
            orig_j = CFT_REMOVED_IDX;                 // mark column to be removed
            for (ridx_t prev_i : inst.cols[prev_j]) {
                removed_rows += fixing.curr2orig.row_map[prev_i] != CFT_REMOVED_IDX ? 1 : 0;
                fixing.curr2orig.row_map[prev_i] = CFT_REMOVED_IDX;  // mark row to be removed
            }
        }
        return removed_rows;
    }

    inline void set_inst_as_empty(Instance& inst, FixingData& fixing, IdxsMaps& prev2curr) {
        inst.cols.clear();
        inst.rows.clear();
        inst.costs.clear();
        inst.solcosts.clear();

        fixing.curr2orig.col_map.clear();
        fixing.curr2orig.row_map.clear();
        prev2curr.row_map.clear();
    }

    // Remove marked rows and make old->new row mapping
    inline void adjust_rows_pos_and_fill_map(Instance&   inst,
                                             FixingData& fixing,
                                             IdxsMaps&   prev2curr) {
        ridx_t old_nrows = inst.rows.size();
        prev2curr.row_map.assign(old_nrows, CFT_REMOVED_IDX);
        ridx_t next_i = 0;
        for (ridx_t prev_i = 0; prev_i < old_nrows; ++prev_i) {

            ridx_t orig_i = fixing.curr2orig.row_map[prev_i];
            if (orig_i == CFT_REMOVED_IDX)
                continue;

            assert(!inst.rows[prev_i].empty());
            if (next_i != prev_i)
                inst.rows[next_i] = std::move(inst.rows[prev_i]);
            fixing.curr2orig.row_map[next_i] = orig_i;
            prev2curr.row_map[prev_i]        = next_i;
            ++next_i;
        }
        fixing.curr2orig.row_map.resize(next_i);
        inst.rows.resize(next_i);
    }

    // Remove marked columns adjusting row indexes and make old->new col mapping
    inline void adjust_cols_pos_and_idxs_and_fill_map(Instance&   inst,
                                                      FixingData& fixing,
                                                      IdxsMaps&   prev2curr) {
        cidx_t old_ncols = inst.cols.size();
        prev2curr.col_map.assign(old_ncols, CFT_REMOVED_IDX);
        cidx_t curr_j = 0;
        size_t n      = 0;
        for (ridx_t prev_j = 0; prev_j < old_ncols; ++prev_j) {

            cidx_t orig_j = fixing.curr2orig.col_map[prev_j];
            if (orig_j == CFT_REMOVED_IDX)
                continue;

            size_t nbeg = n;  // save here to set cols.begs[new_j] later
            for (size_t o = inst.cols.begs[prev_j]; o < inst.cols.begs[prev_j + 1]; ++o) {
                ridx_t prev_i = inst.cols.idxs[o];
                ridx_t curr_i = prev2curr.row_map[prev_i];
                if (curr_i != CFT_REMOVED_IDX)
                    inst.cols.idxs[n++] = curr_i;
            }
            if (n == nbeg)
                continue;

            inst.cols.begs[curr_j]           = nbeg;  // here to not invalidate o begin
            inst.costs[curr_j]               = inst.costs[prev_j];
            inst.solcosts[curr_j]            = inst.solcosts[prev_j];
            prev2curr.col_map[prev_j]        = curr_j;
            fixing.curr2orig.col_map[curr_j] = orig_j;
            ++curr_j;
        }
        inst.cols.begs[curr_j] = n;
        inst.cols.idxs.resize(n);
        inst.cols.begs.resize(curr_j + 1);
        inst.costs.resize(curr_j);
        inst.solcosts.resize(curr_j);
        prev2curr.col_map.resize(curr_j);
    }

    // Adjust column indexes stored in eanch row
    inline void adjust_rows_idxs(Instance& inst, IdxsMaps const& prev2curr) {
        for (auto& row : inst.rows) {
            cidx_t w = 0;
            for (cidx_t r = 0; r < row.size(); ++r) {
                cidx_t curr_j = prev2curr.col_map[row[r]];
                if (curr_j != CFT_REMOVED_IDX)
                    row[w++] = curr_j;
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
                        FixingData&                fixing,
                        IdxsMaps&                  prev2curr) {

    prev2curr.col_map.clear();
    prev2curr.row_map.clear();
    if (cols_to_fix.empty())
        return;

    IF_DEBUG(auto old_inst = inst);

    ridx_t removed_rows = mark_and_update_fixed_elements(inst, cols_to_fix, fixing);
    if (removed_rows == inst.rows.size())  // If all rows were removed, clear everything
        return set_inst_as_empty(inst, fixing, prev2curr);

    // Map old rows and columns to new ones based on whats has been marked
    adjust_rows_pos_and_fill_map(inst, fixing, prev2curr);
    adjust_cols_pos_and_idxs_and_fill_map(inst, fixing, prev2curr);
    adjust_rows_idxs(inst, prev2curr);

    // IF_DEBUG(col_and_rows_check(inst.cols, inst.rows));   // coherent instance
    // IF_DEBUG(mappings_check(old_inst, inst, prev2curr));  // coherent mappings

    // TODO(cava): reductions step (e.g., 1-col rows)
}

}  // namespace cft

#endif /* CFT_SRC_FIXING_FIX_COLUMNS_HPP */
