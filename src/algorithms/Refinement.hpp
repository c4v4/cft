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
#include "core/Chrono.hpp"
#include "core/cft.hpp"
#include "core/coverage.hpp"
#include "fixing/fix_columns.hpp"
#include "subgradient/utils.hpp"

namespace cft {
struct RefinementFixManager {
    static constexpr real_t alpha      = 1.1;
    static constexpr real_t min_fixing = 0.3;

private:
    real_t          fix_fraction = min_fixing;
    real_t          prev_cost    = limits<real_t>::inf();
    Sorter          sorter;
    CoverCounters<> row_coverage;

public:
    inline std::vector<cidx_t> operator()(Instance const&            inst,
                                          std::vector<real_t> const& best_lagr_mult,
                                          Solution const&            best_sol) {

        fix_fraction *= alpha;
        if (best_sol.cost < prev_cost)
            fix_fraction = min_fixing;
        prev_cost = best_sol.cost;

        auto nrows_real   = static_cast<real_t>(inst.rows.size());
        auto nrows_to_fix = static_cast<ridx_t>(nrows_real * fix_fraction);

        assert(best_lagr_mult.size() == inst.rows.size());
        assert(nrows_to_fix <= inst.rows.size());

        ridx_t nrows = inst.rows.size();
        row_coverage.reset(nrows);
        for (cidx_t j : best_sol.idxs)
            row_coverage.cover(inst.cols[j]);

        // TODO(any): matches the paper name, maybe there exist a better name tho
        auto deltas = std::vector<CidxAndCost>();
        deltas.reserve(best_sol.idxs.size());
        for (cidx_t j : best_sol.idxs) {
            real_t delta        = 0.0;
            real_t reduced_cost = inst.costs[j];
            auto   col          = inst.cols[j];
            for (ridx_t i : col) {
                real_t cov = row_coverage[i];
                delta += best_lagr_mult[i] * (cov - 1.0F) / cov;
                reduced_cost -= best_lagr_mult[i];
            }
            delta += max(reduced_cost, 0.0F);
            deltas.push_back({j, delta});
        }
        sorter.sort(deltas, [](CidxAndCost c) { return c.cost; });

        ridx_t covered_rows = 0;
        row_coverage.reset(nrows);
        auto cols_to_fix = std::vector<cidx_t>();
        for (CidxAndCost c : deltas) {
            cidx_t j = c.col;
            covered_rows += row_coverage.cover(inst.cols[j]);
            cols_to_fix.push_back(j);
            if (covered_rows > nrows_to_fix) {
                cols_to_fix.pop_back();
                break;
            }
        }
        return cols_to_fix;
    }
};

constexpr real_t beta = 1.0;

// Complete CFT algorithm (Refinement + call to 3-phase)
inline Solution run(Instance const& orig_inst,
                    prng_t&         rnd,
                    double          tlim,
                    Solution const& warmstart_sol = {}) {

    auto   timer = Chrono<>();
    cidx_t ncols = orig_inst.cols.size();
    ridx_t nrows = orig_inst.rows.size();

    auto inst     = orig_inst;
    auto best_sol = Solution();

    if (!warmstart_sol.idxs.empty())
        best_sol = warmstart_sol;

    auto three_phase        = ThreePhase();
    auto nofix_lagr_mult    = std::vector<real_t>();
    auto prev2curr          = IdxsMaps();
    auto max_cost           = limits<real_t>::max();
    auto fixing             = FixingData();
    auto select_cols_to_fix = RefinementFixManager();
    make_identity_fixing_data(ncols, nrows, fixing);

    for (size_t iter_counter = 0;; ++iter_counter) {

        auto result_3p = three_phase(inst, rnd, tlim - timer.elapsed<sec>());
        if (result_3p.sol.cost + fixing.fixed_cost < best_sol.cost) {
            from_fixed_to_unfixed_sol(result_3p.sol, fixing, best_sol);
            IF_DEBUG(check_solution(orig_inst, best_sol));
        }

        if (iter_counter == 0) {
            nofix_lagr_mult = std::move(result_3p.unfixed_lagr_mult);
            max_cost        = beta * result_3p.unfixed_lb + CFT_EPSILON;
        }

        if (best_sol.cost <= max_cost || inst.rows.empty() || timer.elapsed<sec>() > tlim)
            break;

        inst             = orig_inst;
        auto cols_to_fix = select_cols_to_fix(inst, nofix_lagr_mult, best_sol);
        make_identity_fixing_data(ncols, nrows, fixing);
        fix_columns(inst, cols_to_fix, fixing, prev2curr);

        auto nrows_real  = static_cast<real_t>(orig_inst.rows.size());
        auto fixing_perc = static_cast<real_t>(inst.rows.size()) * 100.0F / nrows_real;
        fmt::print("REFN > Free rows {} ({:.1f}%), best sol {} fixed cost {} time {:.2f}\n",
                   inst.rows.size(),
                   fixing_perc,
                   best_sol.cost,
                   fixing.fixed_cost,
                   timer.elapsed<sec>());
    }

    return best_sol;
}

}  // namespace cft

#endif /* CFT_SRC_ALGORITHMS_REFINEMENT_HPP */
