// SPDX-FileCopyrightText: 2024 Francesco Cavaliere <francescocava95@gmail.com>
// SPDX-License-Identifier: MIT

#ifndef CFT_SRC_FIXING_COLFIXING_HPP
#define CFT_SRC_FIXING_COLFIXING_HPP


#include <vector>

#include "core/Instance.hpp"
#include "core/cft.hpp"
#include "fixing/FixingData.hpp"
#include "greedy/Greedy.hpp"
#include "utils/Chrono.hpp"
#include "utils/CoverCounters.hpp"
#include "utils/print.hpp"

namespace cft {

// 3phase column fixing step of the CFT algorithm.
class ColFixing {

    // Caches
    std::vector<cidx_t> cols_to_fix;    // Columns indexes to fix
    IdxsMaps            old2new;        // Indexes mappings between before/after fixing
    CoverCounters       row_coverage;   // Row coverage for column fixing
    std::vector<real_t> reduced_costs;  // Reduced costs of columns

public:
    // Fixes columns in instance and update the lagrange multipliers.
    void operator()(Environment const&   env,         // in
                    ridx_t               orig_nrows,  // in
                    Instance&            inst,        // inout
                    FixingData&          fixing,      // inout
                    std::vector<real_t>& lagr_mult,   // inout
                    Greedy&              greedy       // cache
    ) {
        assert(rsize(inst.rows) == rsize(fixing.curr2orig.row_map));
        assert(rsize(inst.rows) == rsize(lagr_mult));

        auto timer = Chrono<>();
        _select_non_overlapping_cols(inst, lagr_mult, row_coverage, cols_to_fix, reduced_costs);
        cidx_t no_overlap_ncols = csize(cols_to_fix);

        cidx_t fix_at_least = csize(cols_to_fix) + max(1_C, as_cidx(orig_nrows / 200_R));
        greedy(inst, lagr_mult, reduced_costs, cols_to_fix, limits<real_t>::max(), fix_at_least);

        fix_columns_and_compute_maps(cols_to_fix, inst, fixing, old2new);
        _apply_maps_to_lagr_mult(old2new, lagr_mult);

        print<4>(env,
                 "CFIX> Fixing {} columns ({} + {}), time {:.2f}s\n\n",
                 csize(cols_to_fix),
                 no_overlap_ncols,
                 csize(cols_to_fix) - no_overlap_ncols,
                 timer.elapsed<sec>());
    }

private:
    static void _select_non_overlapping_cols(Instance const&            inst,          // in
                                             std::vector<real_t> const& lagr_mult,     // in
                                             CoverCounters&             row_coverage,  // cache
                                             std::vector<cidx_t>&       cols_to_fix,   // out
                                             std::vector<real_t>&       reduced_costs  // out
    ) {
        static constexpr real_t col_fix_thresh = -0.001_F;

        reduced_costs.resize(csize(inst.cols));
        row_coverage.reset(rsize(inst.rows));
        cols_to_fix.clear();

        for (cidx_t j = 0_C; j < csize(inst.cols); ++j) {
            reduced_costs[j] = inst.costs[j];
            for (ridx_t i : inst.cols[j])
                reduced_costs[j] -= lagr_mult[i];

            if (reduced_costs[j] < col_fix_thresh) {
                cols_to_fix.push_back(j);
                row_coverage.cover(inst.cols[j]);
            }
        }

        for (cidx_t& j : cols_to_fix)
            // If you read the paper and do not understand this line, you are not alone.
            // However, if we fix all non-redundant columns, we fix too much and the
            // algorithm terminates in few terations without finding anything good.
            // Fixing only non-overlapping instead, fixes way less and performs better.
            if (any(inst.cols[j], [&](ridx_t i) { return row_coverage[i] > 1; }))
                j = removed_cidx;
        remove_if(cols_to_fix, [](cidx_t j) { return j == removed_cidx; });
    }

    static void _apply_maps_to_lagr_mult(IdxsMaps const&      old2new,   // in
                                         std::vector<real_t>& lagr_mult  // inout
    ) {

        ridx_t old_nrows = rsize(old2new.row_map);
        ridx_t new_i     = 0_R;
        for (ridx_t old_i = 0_R; old_i < old_nrows; ++old_i)
            if (old2new.row_map[old_i] != removed_ridx) {
                assert(new_i <= old_i);
                assert(new_i == old2new.row_map[old_i]);
                lagr_mult[new_i] = lagr_mult[old_i];
                ++new_i;
            }
        lagr_mult.resize(new_i);
    }
};

}  // namespace cft


#endif /* CFT_SRC_FIXING_COLFIXING_HPP */
