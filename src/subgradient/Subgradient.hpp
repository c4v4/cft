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

#include <fmt/base.h>

#include <cassert>
#include <cstddef>
#include <vector>

#include "core/Instance.hpp"
#include "core/cft.hpp"
#include "greedy/Greedy.hpp"
#include "subgradient/Pricer.hpp"
#include "subgradient/utils.hpp"
#include "utils/Chrono.hpp"
#include "utils/coverage.hpp"
#include "utils/limits.hpp"
#include "utils/utility.hpp"

namespace cft {

// Subgradient phase of the Three-phase algorithm.
class Subgradient {
    // Caches
    Pricer              price;
    Solution            lb_sol;
    CoverCounters<>     row_coverage;
    std::vector<real_t> reduced_costs;
    std::vector<real_t> lagr_mult;

public:
    real_t operator()(Instance const&      orig_inst,      // in
                      real_t               cutoff,         // in
                      Sorter&              sorter,         // cache
                      InstAndMap&          core,           // inout
                      real_t&              step_size,      // inout
                      std::vector<real_t>& best_lagr_mult  // inout
    ) {

        size_t const nrows       = orig_inst.rows.size();
        real_t const max_real_lb = cutoff - CFT_EPSILON;

        assert(!orig_inst.cols.empty() && "Empty instance");
        assert(!core.inst.cols.empty() && "Empty core instance");
        assert(nrows == core.inst.rows.size() && "Incompatible instances");

        auto timer          = Chrono<>();
        auto next_step_size = local::StepSizeManager(20, step_size);
        auto should_exit    = local::ExitConditionManager(300);
        auto should_price   = local::PricingManager(10, std::min(1000UL, nrows / 3));
        auto best_core_lb   = _reset_red_costs_and_lb(core.inst.costs, lb_sol, reduced_costs);
        auto best_real_lb   = limits<real_t>::min();
        lagr_mult           = best_lagr_mult;

        fmt::print("SUBG   > Starting subgradient, LB {:.2f}, UB {:.2f}, cutoff {:.2f}\n",
                   lb_sol.cost,
                   cutoff,
                   max_real_lb);

        size_t max_iters = 10 * nrows;
        for (size_t iter = 0; iter < max_iters && best_real_lb < max_real_lb; ++iter) {

            _update_lbsol_and_reduced_costs(core.inst, lagr_mult, lb_sol, reduced_costs);
            _compute_reduced_row_coverage(core.inst, reduced_costs, sorter, row_coverage, lb_sol);
            real_t norm = _compute_subgrad_sqr_norm(row_coverage);

            if (lb_sol.cost > best_core_lb) {
                IF_DEBUG(fmt::print("SUBG   > New best lower bound: {:.2f}\n", lb_sol.cost));
                best_core_lb   = lb_sol.cost;
                best_lagr_mult = lagr_mult;
            }

            if (norm == 0.0) {
                // TODO(cava): is this check correct with a reduced solution? I don't think so...
                // assert(abs(best_core_lb - lb_sol.cost) / abs(best_core_lb) < 0.01);
                fmt::print("SUBG   > Found optimal solution.\n");
                best_lagr_mult = lagr_mult;
                break;
            }

            if (should_exit(iter, best_core_lb))
                break;

            step_size          = next_step_size(iter, lb_sol.cost);
            real_t step_factor = step_size * (cutoff - lb_sol.cost) / norm;
            _update_lagr_mult(row_coverage, step_factor, lagr_mult);

            if (should_price(iter) && iter < max_iters - 1) {
                real_t real_lb = price(orig_inst, lagr_mult, core);
                should_price.update(best_core_lb, real_lb, cutoff);

                fmt::print("SUBG   > {:4}: Core LB: {:10.2f}  Real LB: {:10.2f}  Step size: "
                           "{:.1}\n",
                           iter,
                           best_core_lb,
                           real_lb,
                           step_size);

                best_real_lb = max(best_real_lb, real_lb);
                best_core_lb = _reset_red_costs_and_lb(core.inst.costs, lb_sol, reduced_costs);
            }
        }

        fmt::print("SUBG   > Subgradient ended in {:.2f}s\n", timer.elapsed<sec>());
        return best_real_lb;
    }

    // Heuristic phase of the Three-phase algorithm.
    // NOTE: It seems that in the original they store the lagrangian multipliers associated to the
    // best lower bound, however, it seems to work better if we store the lagrangian multipliers
    // associated to the best greedy solution. (But this might be due to the different column fixing
    // we are using).
    // TODO(acco): Consider implementing it as a functor.
    void heuristic(Instance const&      inst,           // in
                   real_t               step_size,      // in
                   size_t               max_iters,      // in
                   Greedy&              greedy,         // cache
                   Solution&            best_sol,       // inout
                   std::vector<real_t>& best_lagr_mult  // inout
    ) {

        auto timer        = Chrono<>();
        auto greedy_sol   = Solution();
        auto best_core_lb = _reset_red_costs_and_lb(inst.costs, lb_sol, reduced_costs);
        lagr_mult         = best_lagr_mult;

        for (size_t iter = 0; iter < max_iters; ++iter) {

            _update_lbsol_and_reduced_costs(inst, lagr_mult, lb_sol, reduced_costs);
            row_coverage.reset(inst.rows.size());
            for (cidx_t j : lb_sol.idxs)
                row_coverage.cover(inst.cols[j]);
            real_t norm = _compute_subgrad_sqr_norm(row_coverage);

            if (lb_sol.cost > best_core_lb) {
                best_core_lb   = lb_sol.cost;
                best_lagr_mult = lagr_mult;
            }

            assert(best_core_lb <= best_sol.cost && "Inconsistent lower bound");
            if (best_core_lb >= best_sol.cost - CFT_EPSILON)
                return;

            greedy_sol.idxs.clear();
            greedy(inst, lagr_mult, reduced_costs, greedy_sol, best_sol.cost);
            if (greedy_sol.cost <= best_sol.cost - CFT_EPSILON) {
                best_sol = greedy_sol;
                fmt::print("HEUR   > Improved current solution {:.2f}\n", best_sol.cost);
                IF_DEBUG(check_solution(inst, best_sol));
            }

            if (norm == 0.0) {
                assert(best_core_lb < best_sol.cost && "Optimum is above cutoff");
                fmt::print("HEUR   > Found optimal solution.\n");
                best_lagr_mult = lagr_mult;
                return;
            }

            real_t step_factor = step_size * (best_sol.cost - lb_sol.cost) / norm;
            _update_lagr_mult(row_coverage, step_factor, lagr_mult);
        }

        fmt::print("HEUR   > Heuristic phase ended in {:.2f}s\n", timer.elapsed<sec>());
    }

private:
    // Initialize lower-bounds and reduced costs as if lagr_mult were 0s. NOTE: This function
    // assumes to be called just before the start of a new iteration of the subgradient loop. In
    // this location, reduced_costs and lb_sol have the values corresponding to lagr_mult = 0s,
    // since they are updated at the start.
    static real_t _reset_red_costs_and_lb(std::vector<real_t> const& col_costs,     // in
                                          Solution&                  lb_sol,        // out
                                          std::vector<real_t>&       reduced_costs  // out
    ) {

        // Ready to be updated at the start of the loop
        reduced_costs = col_costs;
        lb_sol.cost   = 0.0;
        lb_sol.idxs.clear();
        return limits<real_t>::min();  // NOTE: avoid 0.0, messes with the step size computation.
    }

    static void _update_lagr_mult(CoverCounters<> const& row_coverage,
                                  real_t                 step_factor,
                                  std::vector<real_t>&   lagr_mult) {

        for (size_t i = 0; i < row_coverage.size(); ++i) {
            auto violation = static_cast<real_t>(1 - row_coverage[i]);

            real_t old_mult   = lagr_mult[i];
            real_t delta_mult = step_factor * violation;
            lagr_mult[i]      = max(0.0F, old_mult + delta_mult);
            assert(std::isfinite(lagr_mult[i]) && "Multiplier is not finite");
        }
    }

    static void _update_lbsol_and_reduced_costs(Instance const&            inst,
                                                std::vector<real_t> const& lagr_mult,
                                                Solution&                  lb_sol,
                                                std::vector<real_t>&       reduced_costs) {
        lb_sol.idxs.clear();
        lb_sol.cost = 0.0;
        for (real_t const value : lagr_mult)
            lb_sol.cost += value;

        for (cidx_t j = 0; j < inst.cols.size(); ++j) {

            reduced_costs[j] = inst.costs[j];
            for (ridx_t i : inst.cols[j])
                reduced_costs[j] -= lagr_mult[i];

            if (reduced_costs[j] < 0.0) {
                lb_sol.idxs.push_back(j);
                lb_sol.cost += reduced_costs[j];
            }
        }
    }

    // Computes the row coverage of the given solution by including the best non-redundant columns.
    static void _compute_reduced_row_coverage(Instance const&            inst,
                                              std::vector<real_t> const& reduced_costs,
                                              Sorter&                    sorter,
                                              CoverCounters<>&           row_coverage,
                                              Solution&                  lb_sol) {

        row_coverage.reset(inst.rows.size());
        sorter.sort(lb_sol.idxs, [&](cidx_t j) { return reduced_costs[j]; });

        for (cidx_t j : lb_sol.idxs) {
            auto col = inst.cols[j];
            if (!row_coverage.is_redundant_cover(col))
                row_coverage.cover(col);
        }
    }

    // Computes the subgradient squared norm according to the given row coverage.
    static real_t _compute_subgrad_sqr_norm(CoverCounters<> const& row_coverage) {
        int32_t norm = 0;
        for (size_t i = 0; i < row_coverage.size(); ++i) {
            int32_t violation = 1 - row_coverage[i];
            norm += violation * violation;
        }
        return static_cast<real_t>(norm);
    }
};
}  // namespace cft


#endif /* CFT_SRC_SUBGRADIENT_SUBGRADIENT_HPP */