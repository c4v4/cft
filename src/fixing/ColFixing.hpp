// Copyright (c) 2024 Francesco Cavaliere
// This program is free software: you can redistribute it and/or modify it under the terms of the
// GNU General Public License as published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version. This program is distributed in the hope that it
// will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details. You should
// have received a copy of the GNU General Public License along with this program. If not, see
// <https://www.gnu.org/licenses/>.

#ifndef CFT_SRC_FIXING_COLFIXING_HPP
#define CFT_SRC_FIXING_COLFIXING_HPP

#include <vector>

#include "core/cft.hpp"
#include "core/coverage.hpp"
#include "fixing/fix_columns.hpp"
#include "greedy/Greedy.hpp"
#include "instance/Instance.hpp"

namespace cft {

// 3phase column fixing step of the CFT algorithm. The original column fixing does not consider the
// current best_solution to further restrict the columns to fix. However, after extensive trials, we
// found that considering the best solution is crucial to achieve comparable results with the
// original. In other words, we were unable to implement the original column fixing technique (as it
// is described in the paper). By considering the current best solution the whole procedure becomes
// quite simpler since we get the redundancy removal for free.
struct ColFixing {
    static constexpr double col_fix_thresh = -0.001;

    // Caches
    std::vector<cidx_t> cols_to_fix;
    CoverCounters<>     cover_counts;

    void operator()(Instance&            inst,
                    FixingData&          fixing,
                    std::vector<real_t>& lagr_mult,
                    std::vector<cidx_t>& best_sol,
                    Greedy&              greedy) {

        ridx_t nrows = inst.rows.size();
        cover_counts.reset(nrows);
        cols_to_fix.clear();

        for (cidx_t j : best_sol) {
            real_t lagr_cost = inst.costs[j];
            for (ridx_t i : inst.cols[j])
                lagr_cost -= lagr_mult[i];

            if (lagr_cost < col_fix_thresh) {
                cols_to_fix.emplace_back(j);
                cover_counts.cover(inst.cols[j]);
            }
        }

        auto fix_at_least = max<cidx_t>(nrows / 200, 1);
        if (fix_at_least > cols_to_fix.size())
            greedy(inst, lagr_mult, cols_to_fix, limits<real_t>::max(), fix_at_least);

        fix_columns(inst, cols_to_fix, fixing);
        // TODO(cava): atm column not in best_sol could be fixed by greedy, can we avoid this?
        ridx_t prev_nrows = fixing.prev2curr_row_map.size();
        for (ridx_t prev_i = 0; prev_i < prev_nrows; ++prev_i) {
            ridx_t curr_i = fixing.prev2curr_row_map[prev_i];
            if (curr_i != CFT_REMOVED_IDX) {
                assert(curr_i <= prev_i);
                lagr_mult[curr_i] = lagr_mult[prev_i];
            }
        }
        lagr_mult.resize(prev_nrows);
    }
};

}  // namespace cft

#endif /* CFT_SRC_FIXING_COLFIXING_HPP */
