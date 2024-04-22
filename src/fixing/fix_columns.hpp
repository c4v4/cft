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
        for (cidx_t old_j = 0_C; old_j < csize(old_inst.cols); ++old_j) {

            cidx_t new_j = old2new.col_map[old_j];
            if (new_j == removed_cidx)
                continue;

            assert(!new_inst.cols[new_j].empty());
            assert(new_inst.cols[new_j].size() <= old_inst.cols[old_j].size());
            for (ridx_t r = 0_R; r < rsize(old_inst.cols[old_j]); ++r) {
                ridx_t old_i = old_inst.cols[old_j][r];
                ridx_t new_i = old2new.row_map[old_i];
                if (new_i == removed_ridx) {
                    assert(any(old_inst.rows[old_i],
                               [&](cidx_t j) { return old2new.col_map[j] == removed_cidx; }));
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

    inline ridx_t compute_maps_from_cols_to_fix(Instance const&            inst,         // in
                                                std::vector<cidx_t> const& cols_to_fix,  // in
                                                IdxsMaps&                  old2new       // out
    ) {

        assert(!cols_to_fix.empty());
        old2new.col_map.assign(csize(inst.cols), 0_C);
        old2new.row_map.assign(rsize(inst.rows), 0_R);

        // Compute the old2new mapping from cols_to_fix.
        // Mark columns and rows to be removed
        ridx_t removed_rows = 0_R;
        for (cidx_t j : cols_to_fix) {
            old2new.col_map[j] = removed_cidx;
            for (ridx_t i : inst.cols[j]) {
                removed_rows += (old2new.row_map[i] != removed_ridx ? 1_R : 0_R);
                old2new.row_map[i] = removed_ridx;
            }
        }
        if (removed_rows == rsize(inst.rows))  // If all rows were removed, return
            return removed_rows;

        // Complete cols mappings for the remaining cols (and remove empty ones)
        cidx_t new_j = 0_C;
        for (cidx_t old_j = 0_C; old_j < csize(old2new.col_map); ++old_j) {
            if (old2new.col_map[old_j] == removed_cidx)
                continue;
            if (any(inst.cols[old_j], [&](ridx_t i) { return old2new.row_map[i] != removed_ridx; }))
                old2new.col_map[old_j] = new_j++;
            else
                old2new.col_map[old_j] = removed_cidx;  // Remove empty columns
        }

        // Complete rows mappings for the remaining rows
        ridx_t n = 0_R;
        for (ridx_t& new_i : old2new.row_map)
            if (new_i != removed_ridx)
                new_i = n++;
        assert(rsize(old2new.row_map) - n == removed_rows);

        return removed_rows;
    }

    inline void inplace_apply_col_map(IdxsMaps const& old2new,  // in
                                      Instance&       inst      // inout
    ) {
        size_t n     = 0;
        cidx_t new_j = 0_C;
        for (cidx_t old_j = 0_C; old_j < csize(inst.cols); ++old_j) {
            if (old2new.col_map[old_j] == removed_cidx)
                continue;

            assert(new_j == old2new.col_map[old_j]);
            size_t new_beg_idx = n;  // save here to set cols.begs[new_j] later
            for (ridx_t old_i : inst.cols[old_j]) {
                ridx_t new_i = old2new.row_map[old_i];
                if (new_i != removed_ridx)
                    inst.cols.idxs[n++] = new_i;
            }

            inst.cols.begs[new_j] = new_beg_idx;  // here to not invalidate current column
            inst.costs[new_j]     = inst.costs[old_j];
            ++new_j;
        }
        cidx_t new_ncols          = new_j;
        inst.cols.begs[new_ncols] = n;
        inst.cols.begs.resize(new_ncols + 1);
        inst.cols.idxs.resize(n);
        inst.costs.resize(new_ncols);
    }

    inline void inplace_apply_row_map(IdxsMaps const& old2new,  // in
                                      Instance&       inst      // inout
    ) {
        ridx_t new_i = 0_R;
        for (ridx_t old_i = 0_R; old_i < rsize(inst.rows); ++old_i) {
            if (old2new.row_map[old_i] == removed_ridx)
                continue;

            auto& old_row = inst.rows[old_i];
            auto& new_row = inst.rows[new_i++];
            new_row.resize(old_row.size());

            cidx_t w = 0_C;
            for (cidx_t old_j : old_row) {
                cidx_t new_j = old2new.col_map[old_j];
                if (new_j != removed_cidx)
                    new_row[w++] = new_j;
            }
            new_row.resize(w);
            assert(!new_row.empty() && "Empty row after fixing -> infeasible core instance");
        }
        inst.rows.resize(new_i);
    }
}  // namespace
}  // namespace local

// Removes a given set of columns from the instance. First the mappings between the old and new
// indexes are computed, then these mappings are appplied.
inline void remove_fixed_cols_from_inst(std::vector<cidx_t> const& cols_to_fix,  // in
                                        Instance&                  inst,         // inout
                                        IdxsMaps&                  old2new       // out
) {

    CFT_IF_DEBUG(auto old_inst = inst);  // save old instance for debug checks

    ridx_t removed_rows = local::compute_maps_from_cols_to_fix(inst, cols_to_fix, old2new);
    if (removed_rows == rsize(inst.rows))
        return clear_inst(inst);

    local::inplace_apply_col_map(old2new, inst);
    local::inplace_apply_row_map(old2new, inst);

    CFT_IF_DEBUG(col_and_rows_check(inst.cols, inst.rows));        // coherent instance
    CFT_IF_DEBUG(local::mappings_check(old_inst, inst, old2new));  // coherent mappings
}

}  // namespace cft


#endif /* CFT_SRC_FIXING_FIX_COLUMNS_HPP */
