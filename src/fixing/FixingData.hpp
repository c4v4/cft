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

#ifndef CAV_SRC_FIXING_FIXINGDATA_HPPCFT_
#define CAV_SRC_FIXING_FIXINGDATA_HPPCFT_

#include "core/Instance.hpp"
#include "core/cft.hpp"
#include "fixing/fix_columns.hpp"

namespace cft {

// Fixing data storing the mappings to the original instance and the original columns indexes and
// costs of the fixed columns.
struct FixingData {
    IdxsMaps            curr2orig;
    std::vector<cidx_t> fixed_cols;
    real_t              fixed_cost = 0.0_F;
};

namespace local { namespace {

    // Invoke BEFORE removing fixed columns from the instance, otherwise `cols_to_fix` indexes will
    // not match the correct columns in `inst` (since they have just been removed).
    inline void add_cols_to_fixing_data(Instance const&            inst,         // in
                                        std::vector<cidx_t> const& cols_to_fix,  // in
                                        FixingData&                fixing        // inout
    ) {
        assert("Size mismatch between fixing data and inst columns" &&
               csize(fixing.curr2orig.col_map) == csize(inst.cols));

        // Use old fixing data to insert fixed columns and costs
        for (cidx_t j : cols_to_fix) {
            assert("Column not in instance" && j < csize(inst.cols));
            cidx_t orig_j = fixing.curr2orig
                                .col_map[j];  // Use old fixing to store original indexes
            assert(orig_j != removed_idx);
            fixing.fixed_cols.push_back(orig_j);
            fixing.fixed_cost += inst.costs[j];
        }
    }

    // Invoke AFTER having removed fixed columns from the instance, since a valid `old2new` mapping
    // must be available.
    inline void apply_maps_to_fixing_data(Instance const& inst,     // in
                                          IdxsMaps const& old2new,  // in
                                          FixingData&     fixing    // inout
    ) {
        cidx_t old_ncols = csize(old2new.col_map);
        ridx_t old_nrows = rsize(old2new.row_map);
        cidx_t new_ncols = csize(inst.cols);
        ridx_t new_nrows = rsize(inst.rows);

        assert("Instance wit fixing has more columns than before" && new_ncols <= old_ncols);
        assert("Instance wit fixing has more rows than before" && new_nrows <= old_nrows);

        // Update original col mappings
        for (cidx_t old_j = 0_C; old_j < old_ncols; ++old_j) {
            cidx_t new_j = old2new.col_map[old_j];
            if (new_j != removed_idx)
                fixing.curr2orig.col_map[new_j] = fixing.curr2orig.col_map[old_j];
        }
        fixing.curr2orig.col_map.resize(new_ncols);

        // Update original row mappings
        for (ridx_t old_i = 0_R; old_i < old_nrows; ++old_i) {
            ridx_t new_i = old2new.row_map[old_i];
            if (new_i != removed_idx)
                fixing.curr2orig.row_map[new_i] = fixing.curr2orig.row_map[old_i];
        }
        fixing.curr2orig.row_map.resize(new_nrows);
    }
}  // namespace
}  // namespace local

// Create a fixing data structure with identity mappings and no fixed columns.
inline void make_identity_fixing_data(cidx_t ncols, ridx_t nrows, FixingData& fixing) {
    fixing.curr2orig.col_map.resize(ncols);
    fixing.curr2orig.row_map.resize(nrows);
    fixing.fixed_cols.clear();
    fixing.fixed_cost = 0.0_F;

    for (cidx_t j = 0_C; j < ncols; ++j)
        fixing.curr2orig.col_map[j] = j;
    for (ridx_t i = 0_R; i < nrows; ++i)
        fixing.curr2orig.row_map[i] = i;
}

// This wrapper is here to avoid the risk of calling the functions in the wrong order.
inline void fix_columns_and_compute_maps(std::vector<cidx_t> const& cols_to_fix,  // in
                                         Instance&                  inst,         // inout
                                         FixingData&                fixing,       // inout
                                         IdxsMaps&                  old2new       // out
) {
    local::add_cols_to_fixing_data(inst, cols_to_fix, fixing);  // Before removing fixed columns
    remove_fixed_cols_from_inst(cols_to_fix, inst, old2new);    // Apply removal
    local::apply_maps_to_fixing_data(inst, old2new, fixing);    // After removing fixed columns
}

}  // namespace cft

#endif /* CAV_SRC_FIXING_FIXINGDATA_HPPCFT_ */
