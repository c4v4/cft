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

#ifndef CFT_SRC_FIXING_COLFIXING_HPP
#define CFT_SRC_FIXING_COLFIXING_HPP


#include <fmt/core.h>

#include <vector>

#include "core/Chrono.hpp"
#include "core/cft.hpp"
#include "core/coverage.hpp"
#include "fixing/fix_columns.hpp"
#include "greedy/Greedy.hpp"
#include "instance/Instance.hpp"

namespace cft {

// 3phase column fixing step of the CFT algorithm.
struct ColFixing {
    static constexpr double col_fix_thresh = -0.001;

    // Caches
    Solution        cols_to_fix;
    CoverCounters<> cover_counts;
    IdxsMaps        prev2curr;

    // Original Column Fixing
    void operator()(Instance&            inst,
                    InstAndMap const&    core,  // TODO(cava): map also core_inst?
                    FixingData&          fixing,
                    std::vector<real_t>& lagr_mult,
                    Greedy&              greedy) {

        auto   timer = Chrono<>();
        ridx_t nrows = inst.rows.size();
        cover_counts.reset(nrows);
        cols_to_fix.idxs.clear();
        IF_DEBUG(cols_to_fix.cost = 0);  // To check that fixing.fixed_cost is correct

        for (cidx_t j = 0; j < core.inst.cols.size(); ++j) {  // inst or core.inst?
            real_t j_red_cost = core.inst.costs[j];
            for (ridx_t i : core.inst.cols[j])
                j_red_cost -= lagr_mult[i];

            if (j_red_cost < col_fix_thresh) {
                IF_DEBUG(cols_to_fix.cost += core.inst.costs[j]);
                cols_to_fix.idxs.push_back(core.col_map[j]);
                cover_counts.cover(core.inst.cols[j]);
            }
        }

        for (cidx_t& j : cols_to_fix.idxs)
            // If you read the paper and do not understand this line, you are not alone.
            // However, if we fix all non-redundant columns, we fix too much and the
            // algorithm terminates in few terations without finding anything good.
            // Fixing only non-overlapping instead, fixes way less and performs better.
            if (any(inst.cols[j], [&](ridx_t i) { return cover_counts[i] > 1; })) {
                IF_DEBUG(cols_to_fix.cost -= inst.costs[j]);
                j = CFT_REMOVED_IDX;
            }
        remove_if(cols_to_fix.idxs, [](cidx_t j) { return j == CFT_REMOVED_IDX; });
        fmt::print("CFIX > Fixing {} non-overlapping columns \n", cols_to_fix.idxs.size());

        _complete_fixing(inst, fixing, prev2curr, lagr_mult, greedy, cols_to_fix);
        fmt::print("CFIX > Fixing ended in {:.2f}s\n", timer.elapsed<sec>());
    }

    // The original column fixing does not consider the current best solution to further restrict
    // the columns to fix. Instead, from the set of column with reduced cost under the threshold, it
    // fixes only non-overlapping columns (or at list, this is our educated guess, look at the other
    // column fixing version for more details). This version instead, uses the current best solution
    // to selectd the columns to fix.
    void operator()(Instance&            inst,
                    InstAndMap const&    core,  // TODO(cava): map also core_inst?
                    FixingData&          fixing,
                    std::vector<real_t>& lagr_mult,
                    Solution&            best_sol,
                    Greedy&              greedy) {

        auto timer = Chrono<>();
        cols_to_fix.idxs.clear();
        for (cidx_t j : best_sol.idxs) {
            real_t lagr_cost = core.inst.costs[j];
            for (ridx_t i : core.inst.cols[j])
                lagr_cost -= lagr_mult[i];

            if (lagr_cost < col_fix_thresh)
                cols_to_fix.idxs.push_back(core.col_map[j]);
        }

        _complete_fixing(inst, fixing, prev2curr, lagr_mult, greedy, cols_to_fix);
        fmt::print("CFIX > Fixing ended in {:.2f}s\n", timer.elapsed<sec>());
    }

private:
    static void _complete_fixing(Instance&            inst,
                                 FixingData&          fixing,
                                 IdxsMaps&            prev2curr,
                                 std::vector<real_t>& lagr_mult,
                                 Greedy&              greedy,
                                 Solution&            cols_to_fix) {

        ridx_t nrows        = inst.rows.size();
        auto   fix_at_least = cols_to_fix.idxs.size() + max<cidx_t>(nrows / 200, 1);
        greedy(inst, lagr_mult, cols_to_fix, limits<real_t>::max(), fix_at_least);

        IF_DEBUG(real_t old_fixed_cost = fixing.fixed_cost);
        fix_columns(inst, cols_to_fix.idxs, fixing, prev2curr);
        assert(fixing.fixed_cost - old_fixed_cost == cols_to_fix.cost);

        ridx_t prev_nrows = prev2curr.row_map.size();
        for (ridx_t prev_i = 0; prev_i < prev_nrows; ++prev_i) {
            ridx_t curr_i = prev2curr.row_map[prev_i];
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
