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

#include "instance/Instance.hpp"

namespace cft {
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

inline void apply_maps_to_fixing_data(Instance const&            inst,
                                      std::vector<cidx_t> const& cols_to_fix,
                                      IdxsMaps const&            prev2curr,
                                      FixingData&                fixing) {

    // Use old fixing data to insert fixed columns and costs
    for (cidx_t j : cols_to_fix) {
        cidx_t orig_j = fixing.curr2orig.col_map[j];
        assert(orig_j != CFT_REMOVED_IDX);
        fixing.fixed_cols.push_back(orig_j);
        fixing.fixed_cost += inst.costs[j];
    }

    // Update original col mappings
    for (cidx_t prev_j = 0; prev_j < prev2curr.col_map.size(); ++prev_j) {
        cidx_t curr_j = prev2curr.col_map[prev_j];
        if (curr_j != CFT_REMOVED_IDX)
            fixing.curr2orig.col_map[curr_j] = fixing.curr2orig.col_map[prev_j];
    }

    // Update original row mappings
    for (ridx_t prev_i = 0; prev_i < prev2curr.row_map.size(); ++prev_i) {
        ridx_t curr_i = prev2curr.row_map[prev_i];
        if (curr_i != CFT_REMOVED_IDX)
            fixing.curr2orig.row_map[curr_i] = fixing.curr2orig.row_map[prev_i];
    }
}

}  // namespace cft

#endif /* CAV_SRC_FIXING_FIXINGDATA_HPPCFT_ */
