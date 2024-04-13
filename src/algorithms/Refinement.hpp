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

namespace cft {
namespace local { namespace {

    // Simply transform a solution of an instance with fixing, to a solution of the original
    // instance without fixing
    inline void from_fixed_to_unfixed_sol(Solution const&   sol,
                                          FixingData const& fixing,
                                          Solution&         best_sol) {
        best_sol.cost = sol.cost + fixing.fixed_cost;
        best_sol.idxs = fixing.fixed_cols;
        for (cidx_t j : sol.idxs)
            best_sol.idxs.push_back(fixing.curr2orig.col_map[j]);
    }

    class RefinementFixManager {
        static constexpr real_t alpha      = 1.1_F;
        static constexpr real_t min_fixing = 0.3_F;

        real_t          fix_fraction = min_fixing;
        real_t          prev_cost    = limits<real_t>::inf();
        CoverCounters<> row_coverage;

    public:
        inline std::vector<cidx_t> operator()(Instance const&            inst,
                                              std::vector<real_t> const& best_lagr_mult,
                                              Solution const&            best_sol) {

            fix_fraction *= alpha;
            if (best_sol.cost < prev_cost)
                fix_fraction = min_fixing;
            prev_cost = best_sol.cost;

            auto nrows_real   = as_real(rsize(inst.rows));
            auto nrows_to_fix = static_cast<ridx_t>(nrows_real * fix_fraction);

            assert(rsize(best_lagr_mult) == rsize(inst.rows));
            assert(nrows_to_fix <= rsize(inst.rows));

            ridx_t nrows = rsize(inst.rows);
            row_coverage.reset(nrows);
            for (cidx_t j : best_sol.idxs)
                row_coverage.cover(inst.cols[j]);

            // TODO(any): matches the paper name, maybe we can find a better name tho
            auto deltas = std::vector<CidxAndCost>();
            deltas.reserve(csize(best_sol.idxs));
            for (cidx_t j : best_sol.idxs) {
                real_t delta        = 0.0_F;
                real_t reduced_cost = inst.costs[j];
                auto   col          = inst.cols[j];
                for (ridx_t i : col) {
                    real_t cov = row_coverage[i];
                    delta += best_lagr_mult[i] * (cov - 1.0_F) / cov;
                    reduced_cost -= best_lagr_mult[i];
                }
                delta += max(reduced_cost, 0.0_F);
                deltas.push_back({j, delta});
            }
            cft::sort(deltas, [](CidxAndCost c) { return c.cost; });

            ridx_t covered_rows = 0_R;
            row_coverage.reset(nrows);
            auto cols_to_fix = std::vector<cidx_t>();
            for (CidxAndCost c : deltas) {
                cidx_t j = c.idx;
                covered_rows += as_ridx(row_coverage.cover(inst.cols[j]));
                cols_to_fix.push_back(j);
                if (covered_rows > nrows_to_fix) {
                    cols_to_fix.pop_back();
                    break;
                }
            }
            return cols_to_fix;
        }
    };
}  // namespace
}  // namespace local

constexpr real_t beta = 1.0_F;

// Complete CFT algorithm (Refinement + call to 3-phase)
inline Solution run(Instance const& orig_inst,
                    prng_t&         rnd,
                    double          tlim,
                    Solution const& warmstart_sol = {}) {

    auto   timer = Chrono<>();
    cidx_t ncols = csize(orig_inst.cols);
    ridx_t nrows = rsize(orig_inst.rows);

    auto inst     = orig_inst;
    auto best_sol = Solution();

    if (!warmstart_sol.idxs.empty())
        best_sol = warmstart_sol;

    auto three_phase        = ThreePhase();
    auto nofix_lagr_mult    = std::vector<real_t>();
    auto old2new            = IdxsMaps();
    auto max_cost           = limits<real_t>::max();
    auto fixing             = FixingData();
    auto select_cols_to_fix = local::RefinementFixManager();
    make_identity_fixing_data(ncols, nrows, fixing);
    for (size_t iter_counter = 0;; ++iter_counter) {

        auto result_3p = three_phase(inst, rnd, tlim - timer.elapsed<sec>());
        if (result_3p.sol.cost + fixing.fixed_cost < best_sol.cost) {
            local::from_fixed_to_unfixed_sol(result_3p.sol, fixing, best_sol);
            CFT_IF_DEBUG(check_solution(orig_inst, best_sol));
        }

        if (iter_counter == 0) {
            nofix_lagr_mult = std::move(result_3p.unfixed_lagr_mult);
            max_cost        = beta * result_3p.unfixed_lb + epsilon;
        }

        if (best_sol.cost <= max_cost || timer.elapsed<sec>() > tlim)
            break;

        inst             = orig_inst;
        auto cols_to_fix = select_cols_to_fix(inst, nofix_lagr_mult, best_sol);
        make_identity_fixing_data(ncols, nrows, fixing);
        fix_columns_and_compute_maps(cols_to_fix, inst, fixing, old2new);

        auto nrows_real  = as_real(rsize(orig_inst.rows));
        auto fixing_perc = as_real(rsize(inst.rows)) * 100.0_F / nrows_real;
        fmt::print("REFN > Free rows {} ({:.1f}%), best sol {} fixed cost {} time {:.2f}\n",
                   rsize(inst.rows),
                   fixing_perc,
                   best_sol.cost,
                   fixing.fixed_cost,
                   timer.elapsed<sec>());

        if (inst.rows.empty() || timer.elapsed<sec>() > tlim)
            break;
    }

    return best_sol;
}

}  // namespace cft


#endif /* CFT_SRC_ALGORITHMS_REFINEMENT_HPP */
