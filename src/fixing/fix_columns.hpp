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

#ifndef CFT_SRC_FIXING_FIX_COLUMNS_HPP
#define CFT_SRC_FIXING_FIX_COLUMNS_HPP


#include <vector>
#ifndef NDEBUG
#include <algorithm>
#endif

#include "core/Instance.hpp"
#include "core/cft.hpp"
#include "utils/utility.hpp"

namespace cft {
namespace local { namespace {
#ifndef NDEBUG
    inline void mappings_check(Instance const& old_inst,
                               Instance const& new_inst,
                               IdxsMaps const& old2new) {
        for (cidx_t old_j = 0; old_j < old_inst.cols.size(); ++old_j) {

            cidx_t new_j = old2new.col_map[old_j];
            if (new_j == CFT_REMOVED_IDX)
                continue;

            assert(!new_inst.cols[new_j].empty());
            assert(new_inst.cols[new_j].size() <= old_inst.cols[old_j].size());
            for (ridx_t r = 0; r < old_inst.cols[old_j].size(); ++r) {
                ridx_t old_i = old_inst.cols[old_j][r];
                ridx_t new_i = old2new.row_map[old_i];
                if (new_i == CFT_REMOVED_IDX) {
                    assert(any(old_inst.rows[old_i],
                               [&](cidx_t j) { return old2new.col_map[j] == CFT_REMOVED_IDX; }));
                    continue;
                }
                assert(std::count(new_inst.cols[new_j].begin(),
                                  new_inst.cols[new_j].end(),
                                  new_i) == 1);
                assert(std::count(new_inst.rows[new_i].begin(),
                                  new_inst.rows[new_i].end(),
                                  new_j) == 1);
                assert(new_inst.rows[new_i].size() <= old_inst.rows[old_i].size());
                assert(!new_inst.rows[new_i].empty());
            }
        }
    }
#endif

    inline ridx_t compute_maps_from_cols_to_fix(Instance const&            inst,
                                                std::vector<cidx_t> const& cols_to_fix,
                                                IdxsMaps&                  old2new) {

        assert(!cols_to_fix.empty());
        old2new.col_map.assign(inst.cols.size(), 0);
        old2new.row_map.assign(inst.rows.size(), 0);

        // Compute the old2new mapping from cols_to_fix.
        // Mark columns and rows to be removed
        ridx_t removed_rows = 0;
        for (cidx_t j : cols_to_fix) {
            old2new.col_map[j] = CFT_REMOVED_IDX;
            for (ridx_t i : inst.cols[j]) {
                removed_rows += old2new.row_map[i] != CFT_REMOVED_IDX ? 1 : 0;
                old2new.row_map[i] = CFT_REMOVED_IDX;
            }
        }
        if (removed_rows == inst.rows.size())  // If all rows were removed, return
            return removed_rows;

        // Complete cols mappings for the remaining cols (and remove empty ones)
        cidx_t new_j = 0;
        for (cidx_t old_j = 0; old_j < old2new.col_map.size(); ++old_j) {
            if (old2new.col_map[old_j] == CFT_REMOVED_IDX)
                continue;
            if (any(inst.cols[old_j],
                    [&](ridx_t i) { return old2new.row_map[i] != CFT_REMOVED_IDX; }))
                old2new.col_map[old_j] = new_j++;
            else
                old2new.col_map[old_j] = CFT_REMOVED_IDX;  // Remove empty columns
        }

        // Complete rows mappings for the remaining rows
        ridx_t n = 0;
        for (ridx_t& new_i : old2new.row_map)
            if (new_i != CFT_REMOVED_IDX)
                new_i = n++;
        assert(old2new.row_map.size() - n == removed_rows);

        return removed_rows;
    }

    inline void inplace_apply_col_map(IdxsMaps const& old2new, Instance& inst) {
        size_t n     = 0;
        cidx_t new_j = 0;
        for (cidx_t old_j = 0; old_j < inst.cols.size(); ++old_j) {
            if (old2new.col_map[old_j] == CFT_REMOVED_IDX)
                continue;

            assert(new_j == old2new.col_map[old_j]);
            size_t new_beg_idx = n;  // save here to set cols.begs[new_j] later
            for (ridx_t old_i : inst.cols[old_j]) {
                ridx_t new_i = old2new.row_map[old_i];
                if (new_i != CFT_REMOVED_IDX)
                    inst.cols.idxs[n++] = new_i;
            }

            inst.cols.begs[new_j] = new_beg_idx;  // here to not invalidate current column
            inst.costs[new_j]     = inst.costs[old_j];
            inst.solcosts[new_j]  = inst.solcosts[old_j];
            ++new_j;
        }
        cidx_t new_ncols          = new_j;
        inst.cols.begs[new_ncols] = n;
        inst.cols.begs.resize(new_ncols + 1);
        inst.cols.idxs.resize(n);
        inst.costs.resize(new_ncols);
        inst.solcosts.resize(new_ncols);
    }

    inline void inplace_apply_row_map(IdxsMaps const& old2new, Instance& inst) {
        ridx_t new_i = 0;
        for (ridx_t old_i = 0; old_i < inst.rows.size(); ++old_i) {
            if (old2new.row_map[old_i] == CFT_REMOVED_IDX)
                continue;

            auto& old_row = inst.rows[old_i];
            auto& new_row = inst.rows[new_i++];
            new_row.resize(old_row.size());

            cidx_t w = 0;
            for (cidx_t old_j : old_row) {
                cidx_t new_j = old2new.col_map[old_j];
                if (new_j != CFT_REMOVED_IDX)
                    new_row[w++] = new_j;
            }
            new_row.resize(w);
            assert(!new_row.empty() && "Empty row after fixing -> infeasible core instance");
        }
        inst.rows.resize(new_i);
    }
}  // namespace
}  // namespace local

inline void remove_fixed_cols_from_inst(std::vector<cidx_t> const& cols_to_fix,  // in
                                        Instance&                  inst,         // inout
                                        IdxsMaps&                  old2new       // out
) {

    IF_DEBUG(auto old_inst = inst);

    ridx_t removed_rows = local::compute_maps_from_cols_to_fix(inst, cols_to_fix, old2new);
    if (removed_rows == inst.rows.size())
        return clear_inst(inst);

    local::inplace_apply_col_map(old2new, inst);
    local::inplace_apply_row_map(old2new, inst);

    IF_DEBUG(col_and_rows_check(inst.cols, inst.rows));        // coherent instance
    IF_DEBUG(local::mappings_check(old_inst, inst, old2new));  // coherent mappings
}

}  // namespace cft


#endif /* CFT_SRC_FIXING_FIX_COLUMNS_HPP */
