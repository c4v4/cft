// Copyright (c) 2024 Luca Accorsi, Francesco Cavaliere
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

#ifndef CFT_SRC_SUBGRADIENT_SUBGRADIENT_HPP
#define CFT_SRC_SUBGRADIENT_SUBGRADIENT_HPP

#include <cassert>
#include <cstddef>
#include <vector>

#include "core/Chrono.hpp"
#include "core/cft.hpp"
#include "core/coverage.hpp"
#include "core/limits.hpp"
#include "core/utility.hpp"
#include "fmt/base.h"
#include "greedy/Greedy.hpp"
#include "instance/Instance.hpp"
#include "subgradient/Pricer.hpp"
#include "subgradient/utils.hpp"

namespace cft {

// Subgradient phase of the Three-phase algorithm.
struct Subgradient {
private:
    // Caches
    Pricer              price;
    Solution            lb_sol;
    CoverCounters<>     row_coverage;
    std::vector<real_t> reduced_costs;
    std::vector<real_t> lagr_mult;

    // Initialize lower-bounds and multipliers as if lagr_mult were 0s and have just been updated to
    // new_lagr_mult.
    // NOTE: This function assumes to be called just before the start of a new iteration of the
    // subgradient loop. In this location, lagr_mult has just been updated (so here they already
    // have the "new" values), while reduced_costs and lb_sol have the values corresponding to
    // lagr_mult = 0s, since they are updated at the start.
    real_t _reset_mult_and_lb(std::vector<real_t> const& col_costs,
                              std::vector<real_t> const& new_lagr_mult) {
        lagr_mult = new_lagr_mult;

        // Ready to be updated at the start of the loop
        reduced_costs = col_costs;
        lb_sol.cost   = 0.0;
        lb_sol.idxs.clear();
        return limits<real_t>::min();  // NOTE: avoid 0.0, messes with the step size computation.
    }

public:
    real_t operator()(Instance const&      orig_inst,
                      InstAndMap&          core,
                      Sorter&              sorter,
                      real_t               cutoff,
                      real_t               best_ub,
                      real_t&              step_size,
                      std::vector<real_t>& best_lagr_mult) {

        size_t const nrows       = orig_inst.rows.size();
        real_t const max_real_lb = cutoff - CFT_EPSILON;

        assert(!orig_inst.cols.empty() && "Empty instance");
        assert(!core.inst.cols.empty() && "Empty core instance");
        assert(nrows == core.inst.rows.size() && "Incompatible instances");

        auto timer          = Chrono<>();
        auto next_step_size = StepSizeManager(20, step_size);
        auto should_exit    = ExitConditionManager(300);
        auto should_price   = PricingManager(10, std::min(1000UL, nrows / 3));
        auto best_core_lb   = _reset_mult_and_lb(core.inst.costs, best_lagr_mult);
        auto best_real_lb   = limits<real_t>::min();

        fmt::print("SUBG > Starting subgradient, LB {:.2f}, UB {:.2f}, cutoff {:.2f}\n",
                   lb_sol.cost,
                   best_ub,
                   max_real_lb);

        size_t max_iters = 10 * nrows;
        for (size_t iter = 0; iter < max_iters && best_real_lb < max_real_lb; ++iter) {

            update_lbsol_and_reduced_costs(core.inst, lagr_mult, lb_sol, reduced_costs);
            compute_reduced_row_coverage(core.inst, reduced_costs, sorter, row_coverage, lb_sol);
            real_t norm = compute_subgradient_norm(row_coverage);

            if (lb_sol.cost > best_core_lb) {
                IF_DEBUG(fmt::print("SUBG > New best lower bound: {:.2f}\n", lb_sol.cost));
                best_core_lb   = lb_sol.cost;
                best_lagr_mult = lagr_mult;
            }

            if (norm == 0.0) {
                // TODO(cava): is this check correct with a reduced solution? I don't think so...
                // assert(abs(best_core_lb - lb_sol.cost) / abs(best_core_lb) < 0.01);
                fmt::print("SUBG > Found optimal solution.\n");
                best_lagr_mult = lagr_mult;
                break;
            }

            if (should_exit(iter, best_core_lb))
                break;

            step_size          = next_step_size(iter, lb_sol.cost);
            real_t step_factor = step_size * (best_ub - lb_sol.cost) / norm;
            update_lagr_mult(row_coverage, step_factor, lagr_mult);

            if (should_price(iter) && iter < max_iters - 1) {
                real_t real_lb = price(orig_inst, lagr_mult, core);
                should_price.update(best_core_lb, real_lb, best_ub);

                fmt::print("SUBG > {:4}: Core LB: {:10.2f}  Real LB: {:10.2f}  Step size: {:.1}\n",
                           iter,
                           best_core_lb,
                           real_lb,
                           step_size);

                best_real_lb = max(best_real_lb, real_lb);
                best_core_lb = _reset_mult_and_lb(core.inst.costs, lagr_mult);
            }
        }

        fmt::print("SUBG > Subgradient ended in {:.2f}s\n", timer.elapsed<sec>());
        return best_real_lb;
    }

    // Heuristic phase of the Three-phase algorithm.
    // NOTE: It seems that in the original they store the lagrangian multipliers associated to the
    // best lower bound, however, it seems to work better if we store the lagrangian multipliers
    // associated to the best greedy solution. (But this might be due to the different column fixing
    // we are using).
    // TODO(acco): Consider implementing it as a functor.
    void heuristic(Instance const&      inst,
                   Greedy&              greedy,
                   real_t               cutoff,
                   real_t               step_size,
                   Solution&            best_sol,
                   std::vector<real_t>& best_lagr_mult) {

        auto timer        = Chrono<>();
        auto greedy_sol   = Solution();
        auto best_core_lb = _reset_mult_and_lb(inst.costs, best_lagr_mult);

        size_t max_iters = 250;  // TODO(all): consider making it a parameter.
        for (size_t iter = 0; iter < max_iters; ++iter) {
            update_lbsol_and_reduced_costs(inst, lagr_mult, lb_sol, reduced_costs);
            compute_row_coverage(inst, lb_sol, row_coverage);
            real_t norm = compute_subgradient_norm(row_coverage);

            if (lb_sol.cost > best_core_lb) {
                best_core_lb   = lb_sol.cost;
                best_lagr_mult = lagr_mult;
            }

            assert(best_core_lb <= best_sol.cost && "Inconsistent lower bound");
            if (best_core_lb >= cutoff - CFT_EPSILON)
                return;

            greedy_sol.idxs.clear();
            greedy(inst, lagr_mult, reduced_costs, greedy_sol, cutoff);
            if (greedy_sol.cost <= cutoff - CFT_EPSILON) {
                cutoff   = greedy_sol.cost;
                best_sol = greedy_sol;
                fmt::print("HEUR > Improved current solution {:.2f}\n", best_sol.cost);
                IF_DEBUG(check_solution(inst, best_sol));
            }

            if (norm == 0.0) {  // Return optimum
                assert(best_core_lb < cutoff && "Optimum is above cutoff");
                fmt::print("HEUR > Found optimal solution.\n");
                best_lagr_mult = lagr_mult;
                return;
            }

            real_t step_factor = step_size * (best_sol.cost - lb_sol.cost) / norm;
            update_lagr_mult(row_coverage, step_factor, lagr_mult);
        }

        fmt::print("HEUR > Heuristic phase ended in {:.2f}s\n", timer.elapsed<sec>());
    }
};


}  // namespace cft

#endif