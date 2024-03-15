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

#include "instance/Instance.hpp"

namespace cft {
namespace {
    /// @brief Mark columns and rows to be removed and update fixed cols and costs
    inline ridx_t mark_and_update_fixed_elements(Instance&                  inst,
                                                 std::vector<cidx_t> const& cols_to_fix) {
        size_t removed_rows = 0;
        for (cidx_t lj : cols_to_fix) {
            cidx_t gj = inst.orig_maps.col_idxs[lj];
            assert("Columns removed twice" && gj != CFT_REMOVED_IDX);

            inst.fixed_cost += inst.costs[lj];              // update fixed cost with new fixing
            inst.fixed_orig_idxs.emplace_back(gj);          // add new fixed indexes
            inst.orig_maps.col_idxs[lj] = CFT_REMOVED_IDX;  // mark column to be removed
            for (ridx_t li : inst.cols[lj]) {
                removed_rows += inst.orig_maps.row_idxs[li] == CFT_REMOVED_IDX ? 0 : 1;
                inst.orig_maps.row_idxs[li] = CFT_REMOVED_IDX;  // mark row to be removed
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
        ridx_t new_li    = 0;
        for (ridx_t old_li = 0; old_li < old_nrows; ++old_li) {
            idx_maps.row_idxs.push_back(old_li);
            if (inst.orig_maps.row_idxs[old_li] != CFT_REMOVED_IDX) {
                inst.orig_maps.row_idxs[new_li] = inst.orig_maps.row_idxs[old_li];
                inst.rows[new_li]               = std::move(inst.rows[old_li]);
                ++new_li;
            }
        }
        inst.orig_maps.row_idxs.resize(new_li);
    }

    /// @brief Remove marked columns adjusting row indexes and make old->new col mapping
    inline void adjust_cols_pos_and_idxs_and_fill_map(Instance& inst, IdxMaps& idx_maps) {
        cidx_t old_ncols = inst.cols.size();
        idx_maps.col_idxs.resize(old_ncols);
        cidx_t new_lj = 0;
        for (ridx_t old_lj = 0; old_lj < old_ncols; ++old_lj) {
            idx_maps.col_idxs[old_lj] = new_lj;
            if (inst.orig_maps.col_idxs[old_lj] == CFT_REMOVED_IDX)
                continue;

            if (new_lj != old_lj) {  // move col forward to new position
                inst.cols.begs[new_lj + 1] = inst.cols.begs[new_lj] + inst.cols[old_lj].size();
                size_t n = inst.cols.begs[new_lj], o = inst.cols.begs[old_lj];
                while (o < inst.cols.begs[old_lj + 1])
                    inst.cols.idxs[n++] = idx_maps.row_idxs[inst.cols.idxs[o++]];
            }
            inst.costs[new_lj]              = inst.costs[old_lj];
            inst.solcosts[new_lj]           = inst.solcosts[old_lj];
            inst.orig_maps.col_idxs[new_lj] = inst.orig_maps.col_idxs[old_lj];
            ++new_lj;
        }
        inst.orig_maps.col_idxs.resize(new_lj);
    }

    /// @brief Adjust column indexes stored in eanch row
    inline void adjust_rows_idxs(Instance& inst, IdxMaps const& idx_maps) {
        for (auto& row : inst.rows)
            for (cidx_t& j : row)
                j = idx_maps.col_idxs[j];
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

    ridx_t removed_rows = mark_and_update_fixed_elements(inst, cols_to_fix);

    // If all rows were removed, clear everything
    if (removed_rows == inst.rows.size()) {
        set_inst_as_empty(inst);
        return;
    }

    // Map old rows and columns to new ones based on whats has been marked
    adjust_rows_pos_and_fill_map(inst, idx_maps);
    adjust_cols_pos_and_idxs_and_fill_map(inst, idx_maps);
    adjust_rows_idxs(inst, idx_maps);
}

inline IdxMaps fix_columns(Instance& inst, std::vector<cidx_t> const& cols_to_fix) {
    auto idx_maps = make_idx_maps();
    fix_columns(inst, cols_to_fix, idx_maps);
    return idx_maps;
}

}  // namespace cft

#endif /* CFT_SRC_FIXING_FIX_COLUMNS_HPP */
