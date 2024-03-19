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
    Fixing              fixing;

    void operator()(Instance&            inst,
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
        ridx_t new_nrows = fixing.new2old_row_map.size();
        for (ridx_t i = 0; i < new_nrows; ++i) {
            assert(i <= fixing.new2old_row_map[i]);
            assert(fixing.new2old_row_map[i] < lagr_mult.size());
            lagr_mult[i] = lagr_mult[fixing.new2old_row_map[i]];
        }
        lagr_mult.resize(new_nrows);
    }
};

inline ColFixing make_col_fixing() {
    return {{}, make_cover_counters(), make_fixing()};
}

}  // namespace cft

#endif /* CFT_SRC_FIXING_COLFIXING_HPP */
