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

#ifndef NDEBUG
#include <algorithm>
#endif

#include "instance/Instance.hpp"

namespace cft {
namespace {
#ifndef NDEBUG
    void mappings_check(Instance const&            old_inst,
                        Instance const&            inst,
                        IdxMaps const&             idx_maps,
                        std::vector<cidx_t> const& fixed_cols) {
        for (cidx_t old_j = 0; old_j < old_inst.cols.size(); ++old_j) {

            cidx_t new_j = idx_maps.col_idxs[old_j];
            if (std::count(fixed_cols.begin(), fixed_cols.end(), old_j) == 1)
                assert(new_j == CFT_REMOVED_IDX);
            if (new_j == CFT_REMOVED_IDX)
                continue;

            assert(!inst.cols[new_j].empty());
            assert(inst.cols[new_j].size() <= old_inst.cols[old_j].size());
            for (ridx_t r = 0; r < inst.cols[new_j].size(); ++r) {
                ridx_t old_i = old_inst.cols[old_j][r];
                ridx_t new_i = idx_maps.row_idxs[old_i];
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

    /// @brief Mark columns and rows to be removed and update fixed cols and costs
    inline ridx_t mark_and_update_fixed_elements(Instance&                  inst,
                                                 std::vector<cidx_t> const& cols_to_fix) {
        size_t removed_rows = 0;
        for (cidx_t j : cols_to_fix) {
            cidx_t& orig_j = inst.orig_maps.col_idxs[j];
            assert("Columns removed twice" && orig_j != CFT_REMOVED_IDX);

            inst.fixed_cost += inst.costs[j];        // update fixed cost with new fixing
            inst.fixed_orig_idxs.push_back(orig_j);  // add new fixed indexes
            orig_j = CFT_REMOVED_IDX;                // mark column to be removed
            for (ridx_t i : inst.cols[j]) {
                removed_rows += inst.orig_maps.row_idxs[i] != CFT_REMOVED_IDX ? 1 : 0;
                inst.orig_maps.row_idxs[i] = CFT_REMOVED_IDX;  // mark row to be removed
            }
        }
        return removed_rows;
    }

    inline void set_inst_as_empty(Instance& inst) {
        inst.cols.clear();
        inst.rows.clear();
        inst.costs.clear();
        inst.solcosts.clear();
        inst.orig_maps.clear();
    }

    /// @brief Remove marked rows and make old->new row mapping
    inline void adjust_rows_pos_and_fill_map(Instance& inst, IdxMaps& idx_maps) {
        ridx_t old_nrows = inst.rows.size();
        idx_maps.row_idxs.assign(old_nrows, CFT_REMOVED_IDX);
        ridx_t new_i = 0;
        for (ridx_t old_i = 0; old_i < old_nrows; ++old_i) {
            ridx_t orig_i = inst.orig_maps.row_idxs[old_i];
            if (orig_i != CFT_REMOVED_IDX) {
                assert(!inst.rows[old_i].empty());
                idx_maps.row_idxs[old_i] = new_i;
                if (new_i != old_i) {
                    inst.rows[new_i]               = std::move(inst.rows[old_i]);
                    inst.orig_maps.row_idxs[new_i] = orig_i;
                }
                ++new_i;
            }
        }
        inst.orig_maps.row_idxs.resize(new_i);
        inst.rows.resize(new_i);
    }

    /// @brief Remove marked columns adjusting row indexes and make old->new col mapping
    inline void adjust_cols_pos_and_idxs_and_fill_map(Instance& inst, IdxMaps& idx_maps) {
        cidx_t old_ncols = inst.cols.size();
        idx_maps.col_idxs.assign(old_ncols, CFT_REMOVED_IDX);
        cidx_t new_j = 0;
        size_t n     = 0;
        for (ridx_t old_j = 0; old_j < old_ncols; ++old_j) {

            cidx_t orig_j = inst.orig_maps.col_idxs[old_j];
            if (orig_j == CFT_REMOVED_IDX)
                continue;

            size_t nbeg = n;  // save here to set cols.begs[new_j] later
            for (size_t o = inst.cols.begs[old_j]; o < inst.cols.begs[old_j + 1]; ++o) {
                ridx_t old_i = inst.cols.idxs[o];
                ridx_t new_i = idx_maps.row_idxs[old_i];
                if (new_i != CFT_REMOVED_IDX)
                    inst.cols.idxs[n++] = new_i;
            }
            if (n == nbeg)
                continue;

            inst.cols.begs[new_j]          = nbeg;  // here to not invalidate o begin
            inst.costs[new_j]              = inst.costs[old_j];
            inst.solcosts[new_j]           = inst.solcosts[old_j];
            inst.orig_maps.col_idxs[new_j] = orig_j;
            idx_maps.col_idxs[old_j]       = new_j;
            ++new_j;
        }
        inst.cols.begs[new_j] = n;
        inst.cols.idxs.resize(n);
        inst.cols.begs.resize(new_j + 1);
        inst.orig_maps.col_idxs.resize(new_j);
    }

    /// @brief Adjust column indexes stored in eanch row
    inline void adjust_rows_idxs(Instance& inst, IdxMaps const& idx_maps) {
        for (auto& row : inst.rows) {
            cidx_t w = 0;
            for (cidx_t r = 0; r < row.size(); ++r) {
                cidx_t new_j = idx_maps.col_idxs[row[r]];
                if (new_j != CFT_REMOVED_IDX)
                    row[w++] = new_j;
            }
            assert(w > 0);
            row.resize(w);
        }
    }
}  // namespace

/// @brief Modifies instance by fixing columns in-place.
/// New indexes are always <= old ones, allowing in-place external data structure updates.
/// Note: Column fixing is irreversible, i.e., you cannot get the original instance from the
/// subinstance.
inline void fix_columns(Instance& inst, std::vector<cidx_t> const& cols_to_fix, IdxMaps& idx_maps) {
    idx_maps.clear();
    if (cols_to_fix.empty())
        return;

#ifndef NDEBUG
    auto old_inst = inst;
#endif

    ridx_t removed_rows = mark_and_update_fixed_elements(inst, cols_to_fix);
    if (removed_rows == inst.rows.size())  // If all rows were removed, clear everything
        return set_inst_as_empty(inst);

    // Map old rows and columns to new ones based on whats has been marked
    adjust_rows_pos_and_fill_map(inst, idx_maps);
    adjust_cols_pos_and_idxs_and_fill_map(inst, idx_maps);
    adjust_rows_idxs(inst, idx_maps);

#ifndef NDEBUG
    inst.invariants_check();  // coherent instance
    mappings_check(old_inst, inst, idx_maps, cols_to_fix);
#endif

    // TODO(cava): reductions step (e.g., 1-col rows)
}

inline IdxMaps fix_columns(Instance& inst, std::vector<cidx_t> const& cols_to_fix) {
    auto idx_maps = make_idx_maps();
    fix_columns(inst, cols_to_fix, idx_maps);
    return idx_maps;
}

}  // namespace cft

#endif /* CFT_SRC_FIXING_FIX_COLUMNS_HPP */
