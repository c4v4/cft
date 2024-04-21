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

#ifndef CFT_SRC_ALGORITHMS_REFINEMENT_HPP
#define CFT_SRC_ALGORITHMS_REFINEMENT_HPP


#include "algorithms/ThreePhase.hpp"
#include "core/cft.hpp"
#include "utils/Chrono.hpp"
#include "utils/coverage.hpp"
#include "utils/utility.hpp"

namespace cft {
namespace local { namespace {

    // Convert a solution from a fixed instance to the original unfixed instance.
    inline void from_fixed_to_unfixed_sol(Solution const&   sol,      // in
                                          FixingData const& fixing,   // in
                                          Solution&         best_sol  // out
    ) {
        best_sol.cost = sol.cost + fixing.fixed_cost;
        best_sol.idxs = fixing.fixed_cols;
        for (cidx_t j : sol.idxs)
            best_sol.idxs.push_back(fixing.curr2orig.col_map[j]);
    }

    class RefinementFixManager {
        static constexpr real_t min_fixing = 0.3_F;

        real_t                   fix_fraction      = 0.0_F;
        real_t                   prev_cost         = limits<real_t>::max();
        CoverCounters<>          row_coverage      = {};
        std::vector<CidxAndCost> gap_contributions = {};  // Delta values in the paper


    public:
        // Finds a set of columns to fix in the next refinement iteration.
        inline std::vector<cidx_t> operator()(Environment const&         env,             // in
                                              Instance const&            inst,            // in
                                              std::vector<real_t> const& best_lagr_mult,  // in
                                              Solution const&            best_sol         // in
        ) {
            ridx_t const nrows = rsize(inst.rows);

            fix_fraction = min(1.0_F, fix_fraction * env.alpha);
            if (best_sol.cost < prev_cost)
                fix_fraction = min_fixing;
            prev_cost = best_sol.cost;

            auto nrows_real   = as_real(rsize(inst.rows));
            auto nrows_to_fix = checked_cast<ridx_t>(nrows_real * fix_fraction);

            assert(rsize(best_lagr_mult) == rsize(inst.rows));
            assert(nrows_to_fix <= rsize(inst.rows));

            row_coverage.reset(nrows);
            for (cidx_t j : best_sol.idxs)
                row_coverage.cover(inst.cols[j]);

            gap_contributions.clear();
            for (cidx_t j : best_sol.idxs) {
                real_t gap_contrib  = 0.0_F;
                real_t reduced_cost = inst.costs[j];
                for (ridx_t i : inst.cols[j]) {
                    real_t cov = as_real(row_coverage[i]);
                    gap_contrib += best_lagr_mult[i] * (cov - 1.0_F) / cov;
                    reduced_cost -= best_lagr_mult[i];
                }
                gap_contrib += max(reduced_cost, 0.0_F);
                gap_contributions.push_back({j, gap_contrib});
            }
            cft::sort(gap_contributions, [](CidxAndCost c) { return c.cost; });

            ridx_t covered_rows = 0_R;
            row_coverage.reset(nrows);
            auto cols_to_fix = std::vector<cidx_t>();
            for (CidxAndCost c : gap_contributions) {
                covered_rows += as_ridx(row_coverage.cover(inst.cols[c.idx]));
                if (covered_rows > nrows_to_fix)
                    break;
                cols_to_fix.push_back(c.idx);
            }
            return cols_to_fix;
        }
    };
}  // namespace
}  // namespace local

// Complete CFT algorithm (Refinement + call to 3-phase)
inline Solution run(Environment const& env,                // in
                    Instance const&    orig_inst,          // in
                    Solution const&    warmstart_sol = {}  // in
) {

    cidx_t const ncols = csize(orig_inst.cols);
    ridx_t const nrows = rsize(orig_inst.rows);

    auto inst     = orig_inst;
    auto best_sol = Solution();

    if (!warmstart_sol.idxs.empty())
        best_sol = warmstart_sol;

    auto three_phase        = ThreePhase();
    auto select_cols_to_fix = local::RefinementFixManager();
    auto nofix_lagr_mult    = std::vector<real_t>();
    auto old2new            = IdxsMaps();
    auto fixing             = FixingData();
    auto max_cost           = limits<real_t>::max();
    make_identity_fixing_data(ncols, nrows, fixing);
    for (size_t iter_counter = 0;; ++iter_counter) {

        auto result_3p = three_phase(env, inst);
        if (result_3p.sol.cost + fixing.fixed_cost < best_sol.cost) {
            local::from_fixed_to_unfixed_sol(result_3p.sol, fixing, best_sol);
            CFT_IF_DEBUG(check_solution(orig_inst, best_sol));
        }

        if (iter_counter == 0) {
            nofix_lagr_mult = std::move(result_3p.unfixed_lagr_mult);
            max_cost        = env.beta * result_3p.unfixed_lb + env.epsilon;
        }

        if (best_sol.cost <= max_cost || env.timer.elapsed<sec>() > env.time_limit)
            break;

        inst             = orig_inst;
        auto cols_to_fix = select_cols_to_fix(env, inst, nofix_lagr_mult, best_sol);
        if (!cols_to_fix.empty()) {
            make_identity_fixing_data(ncols, nrows, fixing);
            fix_columns_and_compute_maps(cols_to_fix, inst, fixing, old2new);
        }
        real_t nrows_real  = as_real(rsize(orig_inst.rows));
        real_t fixing_perc = as_real(rsize(inst.rows)) * 100.0_F / nrows_real;
        print<2>(env,
                 "REFN> {:2}: Best solution {:.2f}, fixed cost {:.2f}, "
                 "fixed rows {:.0f}%, time {:.2f}s\n\n",
                 iter_counter,
                 best_sol.cost,
                 fixing.fixed_cost,
                 fixing_perc,
                 env.timer.elapsed<sec>());

        if (inst.rows.empty() || env.timer.elapsed<sec>() > env.time_limit)
            break;
    }

    return best_sol;
}

}  // namespace cft


#endif /* CFT_SRC_ALGORITHMS_REFINEMENT_HPP */
