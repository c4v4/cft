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
#include "subgradient/utils.hpp"

namespace cft {

constexpr real_t alpha      = 1.1;
constexpr real_t beta       = 1.0;
constexpr real_t min_fixing = 0.3;

inline void select_cols_to_fix(Instance const&            inst,            // in
                               std::vector<real_t> const& best_lagr_mult,  // in
                               Solution const&            best_sol,        // in
                               ridx_t                     nrows_to_fix,    // in
                               Sorter&                    sorter,          // cache
                               CoverCounters<>&           row_coverage,    // cache
                               std::vector<cidx_t>&       cols_to_fix      // out
) {
    ridx_t nrows = inst.rows.size();
    row_coverage.reset(nrows);
    for (cidx_t j : best_sol.idxs)
        row_coverage.cover(inst.cols[j]);

    // TODO(any): matches the paper name, maybe there exist a better name tho
    auto deltas = std::vector<CidxAndCost>();
    deltas.reserve(best_sol.idxs.size());
    for (cidx_t j : best_sol.idxs) {
        auto   col          = inst.cols[j];
        real_t reduced_cost = inst.costs[j];
        for (ridx_t i : col)
            reduced_cost -= best_lagr_mult[i];

        real_t delta = max(reduced_cost, 0.0F);
        for (ridx_t i : col) {
            real_t cov = row_coverage[i];
            delta += best_lagr_mult[i] * (cov - 1.0F) / cov;
        }
        deltas.push_back({j, delta});
    }
    sorter.sort(deltas, [](CidxAndCost c) { return c.cost; });

    ridx_t covered_rows = 0;
    row_coverage.reset(nrows);
    cols_to_fix.clear();
    for (CidxAndCost c : deltas) {
        cidx_t j = c.col;
        covered_rows += row_coverage.cover(inst.cols[j]);
        cols_to_fix.push_back(j);
        if (covered_rows >= nrows_to_fix)
            return;
    }
}

// Complete CFT algorithm (Refinement + call to 3-phase)
inline Solution run(Instance const& orig_inst, prng_t& rnd, Solution const& warmstart_sol = {}) {

    cidx_t ncols = orig_inst.cols.size();
    ridx_t nrows = orig_inst.rows.size();

    auto sorter   = Sorter();
    auto inst     = orig_inst;
    auto best_sol = Solution();

    if (!warmstart_sol.idxs.empty())
        best_sol = warmstart_sol;

    auto   three_phase       = ThreePhase();
    auto   unfixed_lagr_mult = std::vector<real_t>();
    auto   cols_to_fix       = std::vector<cidx_t>();
    auto   prev2curr         = IdxsMaps();
    auto   max_cost          = limits<real_t>::max();
    real_t fix_fraction      = min_fixing;
    size_t iter_counter      = 0;
    auto   fixing            = FixingData();
    make_identity_fixing_data(ncols, nrows, fixing);
    for (;;) {
        auto result_3p = three_phase(inst, rnd);

        fix_fraction *= alpha;
        if (result_3p.sol.cost + fixing.fixed_cost < best_sol.cost) {
            from_fixed_to_unfixed_sol(result_3p.sol, fixing, best_sol);
            IF_DEBUG(check_solution(orig_inst, best_sol));
            fix_fraction = min_fixing;
        }

        if (iter_counter++ == 0) {
            unfixed_lagr_mult = std::move(result_3p.unfixed_lagr_mult);
            max_cost          = beta * result_3p.unfixed_lb + CFT_EPSILON;
        }

        if (best_sol.cost <= max_cost || inst.rows.empty())
            break;

        auto covering_times = CoverCounters<>();
        auto nrows_real     = static_cast<real_t>(nrows);
        auto nrows_to_fix   = static_cast<ridx_t>(nrows_real * fix_fraction);

        make_identity_fixing_data(ncols, nrows, fixing);
        inst = orig_inst;
        select_cols_to_fix(inst,
                           unfixed_lagr_mult,
                           best_sol,
                           nrows_to_fix,
                           sorter,
                           covering_times,
                           cols_to_fix);
        fix_columns(inst, cols_to_fix, fixing, prev2curr);
        fmt::print("REFN > Free rows {} ({:.1f}%), best sol {} fixed cost {}\n",
                   inst.rows.size(),
                   inst.rows.size() * 100.0 / orig_inst.rows.size(),
                   fixing.fixed_cost,
                   best_sol.cost,
                   fixing.fixed_cost);
    }

    return best_sol;
}

}  // namespace cft

#endif /* CFT_SRC_ALGORITHMS_REFINEMENT_HPP */
