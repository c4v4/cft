// SPDX-FileCopyrightText: 2024 Francesco Cavaliere <francescocava95@gmail.com>
// SPDX-License-Identifier: MIT

#ifndef CFT_SRC_SUBGRADIENT_SUBGRADIENT_HPP
#define CFT_SRC_SUBGRADIENT_SUBGRADIENT_HPP


#include <cstddef>
#include <vector>

#include "core/Instance.hpp"
#include "core/cft.hpp"
#include "greedy/Greedy.hpp"
#include "subgradient/Pricer.hpp"
#include "subgradient/utils.hpp"
#include "utils/Chrono.hpp"
#include "utils/CoverCounters.hpp"
#include "utils/assert.hpp"  // IWYU pragma:  keep
#include "utils/limits.hpp"
#include "utils/print.hpp"
#include "utils/utility.hpp"

namespace cft {

// Subgradient phase of the 3-phase algorithm.
class Subgradient {
    // Caches
    Solution            lb_sol;         // Partial solution with negative reduced costs
    Solution            greedy_sol;     // Greedy solution
    CoverCounters       row_coverage;   // Row coverage
    std::vector<real_t> reduced_costs;  // Reduced costs vector
    std::vector<real_t> lagr_mult;      // Lagrangian multipliers

public:
    real_t operator()(Environment const&   env,            // in
                      Instance const&      orig_inst,      // in
                      real_t               cutoff,         // in
                      Pricer               price,          // cache
                      InstAndMap&          core,           // inout
                      real_t&              step_size,      // inout
                      std::vector<real_t>& best_lagr_mult  // inout
    ) {
        size_t const nrows       = size(orig_inst.rows);
        real_t const max_real_lb = cutoff - env.epsilon;

        assert(!orig_inst.cols.empty() && "Empty instance");
        assert(!core.inst.cols.empty() && "Empty core instance");
        assert(nrows == size(core.inst.rows) && "Incompatible instances");

        auto   timer          = Chrono<>();
        auto   next_step_size = local::StepSizeManager(20, step_size);
        auto   should_exit    = local::ExitConditionManager(300);
        auto   should_price   = local::PricingManager(10ULL, min(1000ULL, nrows / 3ULL));
        real_t best_core_lb   = limits<real_t>::min();
        auto   best_real_lb   = limits<real_t>::min();
        _reset_red_costs_and_lb(core.inst.costs, lb_sol, reduced_costs, best_core_lb);
        lagr_mult = best_lagr_mult;

        print<4>(env, "SUBG> Subgradient start: UB {:.2f}, cutoff {:.2f}\n", cutoff, max_real_lb);

        size_t max_iters = 10ULL * nrows;
        for (size_t iter = 0; iter < max_iters && best_real_lb < max_real_lb; ++iter) {

            _update_lbsol_and_reduced_costs(core.inst, lagr_mult, lb_sol, reduced_costs);
            _compute_reduced_row_coverage(core.inst, reduced_costs, row_coverage, lb_sol);
            real_t sqr_norm = _compute_subgrad_sqr_norm(row_coverage);

            if (lb_sol.cost > best_core_lb) {
                print<5>(env, "SUBG> {:4}: Current lower bound: {:.2f}\n", iter, lb_sol.cost);
                best_core_lb   = lb_sol.cost;
                best_lagr_mult = lagr_mult;
            }

            if (sqr_norm < 0.999_F) {  // Squared norm is an integer
                print<4>(env, "SUBG> {:4}: Found optimal solution.\n", iter);
                best_lagr_mult = lagr_mult;
                break;
            }

            if (should_exit(env, iter, best_core_lb))
                break;

            step_size          = next_step_size(iter, lb_sol.cost);
            real_t step_factor = step_size * (cutoff - lb_sol.cost) / sqr_norm;
            _update_lagr_mult(row_coverage, step_factor, lagr_mult);

            if (should_price(iter) && iter < max_iters - 1) {
                real_t real_lb = price(orig_inst, lagr_mult, core);
                should_price.update(best_core_lb, real_lb, cutoff);

                print<4>(env,
                         "SUBG> {:4}: LB: {:8.2f}  Core LB: {:8.2f}  Step size: {:6.1}\n",
                         iter,
                         real_lb,
                         best_core_lb,
                         step_size);

                best_real_lb = max(best_real_lb, real_lb);
                _reset_red_costs_and_lb(core.inst.costs, lb_sol, reduced_costs, best_core_lb);

                if (env.timer.elapsed<sec>() > env.time_limit)
                    break;
            }
        }

        print<4>(env, "SUBG> Subgradient ended in {:.2f}s\n\n", timer.elapsed<sec>());
        return best_real_lb;
    }

    // Heuristic phase of the Three-phase algorithm.
    // NOTE: It seems that in the original they store the lagrangian multipliers associated to the
    // best lower bound, however, it seems to work better if we store the lagrangian multipliers
    // associated to the best greedy solution. (But this might be due to the different column fixing
    // we are using).
    void heuristic(Environment const&   env,            // in
                   Instance const&      core_inst,      // in
                   real_t               step_size,      // in
                   Greedy&              greedy,         // cache
                   Solution&            best_sol,       // inout
                   std::vector<real_t>& best_lagr_mult  // inout
    ) {
        auto   timer        = Chrono<>();
        real_t best_core_lb = limits<real_t>::min();
        _reset_red_costs_and_lb(core_inst.costs, lb_sol, reduced_costs, best_core_lb);
        lagr_mult = best_lagr_mult;

        for (size_t iter = 0; iter < env.heur_iters; ++iter) {

            _update_lbsol_and_reduced_costs(core_inst, lagr_mult, lb_sol, reduced_costs);
            row_coverage.reset(rsize(core_inst.rows));
            for (cidx_t j : lb_sol.idxs)
                row_coverage.cover(core_inst.cols[j]);
            real_t sqr_norm = _compute_subgrad_sqr_norm(row_coverage);

            if (lb_sol.cost > best_core_lb) {
                best_core_lb   = lb_sol.cost;
                best_lagr_mult = lagr_mult;
            }

            real_t cutoff = best_sol.cost;
            if (best_core_lb >= best_sol.cost - env.epsilon)
                return;

            greedy_sol.idxs.clear();
            greedy_sol.cost = greedy(core_inst, lagr_mult, reduced_costs, greedy_sol.idxs, cutoff);
            print<5>(env, "HEUR> {:4}: Greedy solution {:.2f}\n", iter, best_sol.cost);
            if (greedy_sol.cost <= best_sol.cost - env.epsilon) {
                best_sol = greedy_sol;
                print<4>(env, "HEUR> {:4}: Improved solution {:.2f}\n", iter, best_sol.cost);
                CFT_IF_DEBUG(check_inst_solution(core_inst, best_sol));
            }

            if (sqr_norm < 0.999_F) {  // Squared norm is an integer
                assert(best_core_lb <= best_sol.cost && "Optimum is above cutoff");
                print<4>(env, "HEUR> {:4} Found optimal solution.\n", iter);
                best_lagr_mult = lagr_mult;
                return;
            }

            real_t step_factor = step_size * (best_sol.cost - lb_sol.cost) / sqr_norm;
            _update_lagr_mult(row_coverage, step_factor, lagr_mult);

            if (env.timer.elapsed<sec>() > env.time_limit)
                break;
        }

        print<4>(env, "HEUR> Heuristic phase ended in {:.2f}s\n\n", timer.elapsed<sec>());
    }

private:
    // Initialize lower-bounds and reduced costs as if lagr_mult were 0s. NOTE: This function
    // assumes to be called just before the start of a new iteration of the subgradient loop. In
    // this location, reduced_costs and lb_sol have the values corresponding to lagr_mult = 0s,
    // since they are updated at the start.
    static void _reset_red_costs_and_lb(std::vector<real_t> const& col_costs,      // in
                                        Solution&                  lb_sol,         // out
                                        std::vector<real_t>&       reduced_costs,  // out
                                        real_t&                    best_core_lb    // out
    ) {
        // Ready to be updated at the start of the loop
        reduced_costs = col_costs;
        best_core_lb  = limits<real_t>::min();
        lb_sol.cost   = limits<real_t>::min();
        lb_sol.idxs.clear();
    }

    static void _update_lagr_mult(CoverCounters const& row_coverage,  // in
                                  real_t               step_factor,   // in
                                  std::vector<real_t>& lagr_mult      // inout
    ) {
        for (ridx_t i = 0_R; i < rsize(row_coverage); ++i) {
            auto violation = 1.0_F - as_real(row_coverage[i]);

            real_t old_mult   = lagr_mult[i];
            real_t delta_mult = step_factor * violation;
            lagr_mult[i]      = max(0.0_F, old_mult + delta_mult);
            assert(std::isfinite(lagr_mult[i]) && "Multiplier is not finite");
        }
    }

    static void _update_lbsol_and_reduced_costs(Instance const&            inst,          // in
                                                std::vector<real_t> const& lagr_mult,     // in
                                                Solution&                  lb_sol,        // out
                                                std::vector<real_t>&       reduced_costs  // out
    ) {
        lb_sol.idxs.clear();
        lb_sol.cost = 0.0_F;
        for (real_t const value : lagr_mult)
            lb_sol.cost += value;

        for (cidx_t j = 0_C; j < csize(inst.cols); ++j) {

            reduced_costs[j] = inst.costs[j];
            for (ridx_t i : inst.cols[j])
                reduced_costs[j] -= lagr_mult[i];

            if (reduced_costs[j] < 0.0_F) {
                lb_sol.idxs.push_back(j);
                lb_sol.cost += reduced_costs[j];
            }
        }
    }

    // Computes the row coverage of the given solution by including the best non-redundant columns.
    static void _compute_reduced_row_coverage(Instance const&            inst,           // in
                                              std::vector<real_t> const& reduced_costs,  // in
                                              CoverCounters&             row_coverage,   // out
                                              Solution&                  lb_sol          // out
    ) {
        row_coverage.reset(rsize(inst.rows));
        cft::sort(lb_sol.idxs, [&](cidx_t j) { return reduced_costs[j]; });

        for (cidx_t j : lb_sol.idxs) {
            auto col = inst.cols[j];
            if (!row_coverage.is_redundant_cover(col))
                row_coverage.cover(col);
        }
    }

    // Computes the subgradient squared sqr_norm according to the given row coverage.
    static real_t _compute_subgrad_sqr_norm(CoverCounters const& row_coverage) {
        int64_t sqr_norm = 0;
        for (ridx_t i = 0_R; i < rsize(row_coverage); ++i) {
            int64_t violation = 1 - checked_cast<int64_t>(row_coverage[i]);
            sqr_norm += violation * violation;
        }
        return as_real(sqr_norm);
    }
};
}  // namespace cft


#endif /* CFT_SRC_SUBGRADIENT_SUBGRADIENT_HPP */
