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
#include "fixing/FixingData.hpp"
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
    IdxsMaps        prev2curr;  // Indexes mappings between before/after fixing instances

    // Original Column Fixing
    void operator()(Instance&            inst,       // inout
                    InstAndMap&          core,       // inout
                    FixingData&          fixing,     // inout
                    std::vector<real_t>& lagr_mult,  // inout
                    Greedy&              greedy      // cache
    ) {
        assert(inst.rows.size() == core.inst.rows.size());

        auto   timer = Chrono<>();
        ridx_t nrows = inst.rows.size();
        cover_counts.reset(nrows);
        cols_to_fix.idxs.clear();
        cols_to_fix.cost = 0.0;

        _select_non_overlapping_cols(core.inst, lagr_mult, cover_counts, cols_to_fix);
        fmt::print("CFIX > Fixing {} non-overlapping columns \n", cols_to_fix.idxs.size());

        auto fix_at_least = cols_to_fix.idxs.size() + max<cidx_t>(nrows / 200, 1);
        greedy(core.inst, lagr_mult, cols_to_fix, limits<real_t>::max(), fix_at_least);

        // First fix column in core-instance
        fix_columns(core.inst, cols_to_fix.idxs, prev2curr);

        // Map columns to fix into their original indexes
        for (cidx_t& j : cols_to_fix.idxs)
            j = core.col_map[j];

        // Fix them into instance and computing fixing data and mappings
        fix_columns(inst, cols_to_fix.idxs, prev2curr);
        apply_maps_to_fixing_data(inst, cols_to_fix.idxs, prev2curr, fixing);
        _apply_maps_to_lagr_mult(prev2curr, lagr_mult);

        fmt::print("CFIX > Fixing ended in {:.2f}s\n", timer.elapsed<sec>());
    }


private:
    static void _select_non_overlapping_cols(Instance const&            inst,
                                             std::vector<real_t> const& lagr_mult,
                                             CoverCounters<>&           cover_counts,
                                             Solution&                  cols_to_fix) {
        for (cidx_t j = 0; j < inst.cols.size(); ++j) {
            real_t j_red_cost = inst.costs[j];
            for (ridx_t i : inst.cols[j])
                j_red_cost -= lagr_mult[i];

            if (j_red_cost < col_fix_thresh) {
                cols_to_fix.cost += inst.costs[j];
                cols_to_fix.idxs.push_back(j);
                cover_counts.cover(inst.cols[j]);
            }
        }

        for (cidx_t& j : cols_to_fix.idxs)
            // If you read the paper and do not understand this line, you are not alone.
            // However, if we fix all non-redundant columns, we fix too much and the
            // algorithm terminates in few terations without finding anything good.
            // Fixing only non-overlapping instead, fixes way less and performs better.
            if (any(inst.cols[j], [&](ridx_t i) { return cover_counts[i] > 1; })) {
                cols_to_fix.cost -= inst.costs[j];
                j = CFT_REMOVED_IDX;
            }
        remove_if(cols_to_fix.idxs, [](cidx_t j) { return j == CFT_REMOVED_IDX; });
    }

    static void _apply_maps_to_lagr_mult(IdxsMaps const&      prev2curr,
                                         std::vector<real_t>& lagr_mult) {

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
