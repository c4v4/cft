// Copyright (c) 2024 Luca Accorsi and Francesco Cavaliere
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
#include "core/coverage.hpp"
#include "fixing/fix_columns.hpp"

namespace cft {

constexpr real_t alpha  = 1.1;
constexpr real_t beta   = 1.0;
constexpr real_t pi_min = 0.3;
constexpr real_t pi_max = 0.9;

inline void select_cols_to_fix(Instance const&            inst,
                               std::vector<real_t> const& best_lagr_mult,
                               Solution&                  best_sol,
                               ridx_t                     nrows_to_fix,
                               Sorter&                    sorter,
                               std::vector<cidx_t>&       cols_to_fix,
                               CoverCounters<>&           row_coverage) {

    ridx_t nrows = inst.rows.size();
    row_coverage.reset(nrows);
    for (cidx_t j : best_sol.idxs)
        row_coverage.cover(inst.cols[j]);

    auto deltas = std::vector<real_t>();
    for (cidx_t j : best_sol.idxs) {
        real_t reduced_cost = inst.costs[j];
        auto   col          = inst.cols[j];
        for (ridx_t i : col)
            reduced_cost -= best_lagr_mult[i];

        deltas[j] = max(reduced_cost, 0.0F);
        for (ridx_t i : col) {
            real_t cov = row_coverage[i];
            deltas[j] += best_lagr_mult[i] * (cov - 1.0F) / cov;
        }
    }
    sorter.sort(best_sol.idxs, [&](cidx_t j) { return deltas[j]; });

    ridx_t covered_rows = 0;
    row_coverage.reset(nrows);
    for (cidx_t j : best_sol.idxs) {
        covered_rows += row_coverage.cover(inst.cols[j]);
        cols_to_fix.push_back(j);
        if (covered_rows < nrows_to_fix)
            return;
    }
}

// Complete CFT algorithm (Refinement + call to 3-phase)
inline Solution run(Instance& inst, prng_t& rnd, Solution const& warmstart_sol = {}) {

    auto best_sol    = Solution();
    auto cols_to_fix = std::vector<cidx_t>();

    if (!warmstart_sol.idxs.empty())
        best_sol = warmstart_sol;

    auto best_lagr_mult = std::vector<real_t>();
    auto best_lb        = limits<real_t>::min();

    real_t fixing_fraction = pi_min;
    auto   three_phase     = ThreePhase();
    auto   fixing          = FixingData();
    auto   prev2curr       = IdxsMaps();
    size_t iter            = 0;
    auto   sorter          = Sorter();
    for (;;) {
        auto   sol       = three_phase(inst, rnd);
        auto   lagr_mult = std::vector<real_t>();  // TODO(any): 3phase should return also lagr_mult
        real_t lower_bound = 0.0;  // TODO(any): 3phase should return also +lower_bound

        fixing_fraction *= alpha;
        if (sol.cost < best_sol.cost) {
            best_sol        = sol;
            fixing_fraction = max(fixing_fraction / (alpha * alpha), pi_min);  // 6.
            best_lb         = limits<real_t>::min();
        }

        if (iter++ == 0)
            best_lagr_mult = std::move(lagr_mult);

        if (lower_bound > best_lb)
            best_lb = lower_bound;

        if (best_sol.cost - CFT_EPSILON <= beta * best_lb || fixing_fraction > pi_max ||
            inst.rows.empty())
            break;

        fixing_fraction = std::min(pi_max, fixing_fraction);

        auto covering_times = CoverCounters<>();
        auto nrows_real     = static_cast<real_t>(inst.rows.size());
        auto nrows_to_fix   = static_cast<ridx_t>(nrows_real * fixing_fraction);
        select_cols_to_fix(inst,
                           best_lagr_mult,
                           best_sol,
                           nrows_to_fix,
                           sorter,
                           cols_to_fix,
                           covering_times);
        fix_columns(inst, cols_to_fix, fixing, prev2curr);
    }

    return best_sol;
}

}  // namespace cft

#endif /* CFT_SRC_ALGORITHMS_REFINEMENT_HPP */
