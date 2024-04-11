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

#include "core/Instance.hpp"
#include "core/cft.hpp"
#include "fixing/FixingData.hpp"
#include "greedy/Greedy.hpp"
#include "utils/Chrono.hpp"
#include "utils/coverage.hpp"

namespace cft {

// 3phase column fixing step of the CFT algorithm.
class ColFixing {

    // Caches
    Solution            cols_to_fix;
    CoverCounters<>     cover_counts;
    IdxsMaps            old2new;  // Indexes mappings between before/after fixing
    std::vector<real_t> reduced_costs;

public:
    // Original Column Fixing
    void operator()(ridx_t               orig_nrows,
                    Instance&            inst,       // inout
                    FixingData&          fixing,     // inout
                    std::vector<real_t>& lagr_mult,  // inout
                    Greedy&              greedy      // cache
    ) {
        // assert(rsize(inst.rows) == rsize(core.inst.rows));
        assert(rsize(inst.rows) == rsize(fixing.curr2orig.row_map));
        assert(rsize(inst.rows) == rsize(lagr_mult));

        auto timer = Chrono<>();
        _select_non_overlapping_cols(inst, lagr_mult, cover_counts, cols_to_fix, reduced_costs);
        cidx_t no_overlap_ncols = csize(cols_to_fix.idxs);

        auto fix_at_least = csize(cols_to_fix.idxs) + max<cidx_t>(1_C, orig_nrows / 200);
        greedy(inst, lagr_mult, reduced_costs, cols_to_fix, limits<real_t>::max(), fix_at_least);

        fix_columns_and_compute_maps(cols_to_fix.idxs, inst, fixing, old2new);
        apply_maps_to_lagr_mult(old2new, lagr_mult);

        fmt::print("CFIX   > Fixing {} columns ({} + {}), time {:.2f}s\n",
                   csize(cols_to_fix.idxs),
                   no_overlap_ncols,
                   csize(cols_to_fix.idxs) - no_overlap_ncols,
                   timer.elapsed<sec>());
    }

private:
    static void _select_non_overlapping_cols(Instance const&            inst,
                                             std::vector<real_t> const& lagr_mult,
                                             CoverCounters<>&           cover_counts,
                                             Solution&                  cols_to_fix,
                                             std::vector<real_t>&       reduced_costs) {
        static constexpr real_t col_fix_thresh = -0.001_F;

        reduced_costs.resize(csize(inst.cols));
        cover_counts.reset(rsize(inst.rows));
        cols_to_fix.idxs.clear();
        cols_to_fix.cost = 0.0_F;

        for (cidx_t j = 0_C; j < csize(inst.cols); ++j) {
            reduced_costs[j] = inst.costs[j];
            for (ridx_t i : inst.cols[j])
                reduced_costs[j] -= lagr_mult[i];

            if (reduced_costs[j] < col_fix_thresh) {
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
                j = removed_idx;
            }
        remove_if(cols_to_fix.idxs, [](cidx_t j) { return j == removed_idx; });
    }
};

}  // namespace cft


#endif /* CFT_SRC_FIXING_COLFIXING_HPP */
