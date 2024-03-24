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
#include <type_traits>
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

#ifndef NDEBUG
// TODO(any): find a better place for this function.
inline void check_solution(cft::Instance const& inst, cft::Solution const& sol) {
    cft::ridx_t nrows = inst.rows.size();

    // check coverage
    cft::ridx_t covered_rows = 0;
    auto        cover_bits   = cft::CoverBits(nrows);
    for (auto j : sol.idxs)
        covered_rows += cover_bits.cover(inst.cols[j]);
    assert(covered_rows == nrows);

    // check cost
    cft::real_t total_cost = 0;
    for (cft::cidx_t j : sol.idxs)
        total_cost += inst.costs[j];
    assert(std::abs(total_cost - sol.cost) < 1e-6);
}
#endif

// Step size manager functor.
struct StepSizeManager {
    size_t period;
    size_t next_update_iter;
    real_t curr_step_size;
    real_t min_lower_bound;
    real_t max_lower_bound;

    StepSizeManager(size_t c_period, real_t c_init_step_size)
        : period(c_period)
        , next_update_iter(c_period)
        , curr_step_size(c_init_step_size)
        , min_lower_bound(limits<real_t>::max())
        , max_lower_bound(limits<real_t>::min()) {
    }

    // Computes the next step size.
    CFT_NODISCARD real_t operator()(size_t iter, real_t lower_bound) {
        min_lower_bound = cft::min(min_lower_bound, lower_bound);
        max_lower_bound = cft::max(max_lower_bound, lower_bound);
        if (iter == next_update_iter) {
            next_update_iter += period;
            real_t const diff = (max_lower_bound - min_lower_bound) / max_lower_bound;
            if (diff > 0.01)
                curr_step_size /= 2.0;
            else if (diff <= 0.001)
                curr_step_size *= 1.5;
            min_lower_bound = limits<real_t>::max();
            max_lower_bound = limits<real_t>::min();
        }
        return curr_step_size;
    }
};

struct ExitConditionManager {
    size_t period;
    size_t next_update_iter;
    real_t prev_lower_bound;

    ExitConditionManager(size_t c_period)
        : period(c_period)
        , next_update_iter(c_period)
        , prev_lower_bound(limits<real_t>::min()) {
    }

    // Evaluates the exit condition by comparing the current best lower-bound with the
    // previous period's best lower-bound. Returns the original CFT exit condition based on the
    // absolute and relative improvement in the lower-bound.
    CFT_NODISCARD bool operator()(size_t iter, real_t lower_bound) {
        if (iter == next_update_iter) {
            next_update_iter += period;
            real_t abs_improvement      = lower_bound - prev_lower_bound;
            real_t relative_improvement = abs_improvement / lower_bound;
            prev_lower_bound            = lower_bound;
            return abs_improvement < 1.0 && relative_improvement < 0.001;
        }
        return false;
    }
};

// Functor managing the pricing frequency.
struct PricingManager {
    size_t period;
    size_t next_update_iter;
    size_t max_period_increment;

    PricingManager(size_t c_period, size_t c_max_period_increment)
        : period(c_period)
        , next_update_iter(c_period)
        , max_period_increment(c_max_period_increment) {
    }

    bool operator()(size_t iter) const {
        return iter == next_update_iter;
    }

    void update(real_t core_lb, real_t real_lb, real_t ub) {
        real_t const delta = (core_lb - real_lb) / ub;
        if (delta <= 1e-6)
            period = std::min(max_period_increment, 10 * period);
        else if (delta <= 0.02)
            period = std::min(max_period_increment, 5 * period);
        else if (delta <= 0.2)
            period = std::min(max_period_increment, 2 * period);
        else
            period = 10;

        next_update_iter += period;
    }
};

// Computes a solution by inspection by including all columns having a negative reduced cost.
// TODO(cava): Here to save it in the repo, remove it!
// inline void update_lbsol_and_reduced_costs(Instance const&            inst,
//                                            std::vector<real_t> const& lagr_mult_delta,
//                                            Solution&                  lb_sol,
//                                            std::vector<real_t>&       reduced_costs) {
//     lb_sol.idxs.clear();
//     for (real_t value : lagr_mult_delta)
//         lb_sol.cost += value;
//
//     // Update reduced costs
//     for (ridx_t i = 0; i < inst.rows.size(); ++i)
//         if (lagr_mult_delta[i] != 0.0)  // exact match since caused by integral violation = 0
//             for (auto j : inst.rows[i]) {
//                 real_t old_rc = reduced_costs[j];
//                 reduced_costs[j] -= lagr_mult_delta[i];
//                 lb_sol.cost += min(0.0F, reduced_costs[j]) - min(0.0F, old_rc);
//             }
//
//     for (cidx_t j = 0; j < inst.cols.size(); ++j)
//         if (reduced_costs[j] < 0.0)
//             lb_sol.idxs.push_back(j);
// }

inline void update_lbsol_and_reduced_costs(Instance const&            inst,
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

// Computes the row coverage of the given solution.
inline void compute_row_coverage(Instance const&  inst,
                                 Solution const&  sol,
                                 CoverCounters<>& row_coverage) {
    // TODO(acco): consider moving to member.
    row_coverage.reset(inst.rows.size());
    for (cidx_t j : sol.idxs)
        row_coverage.cover(inst.cols[j]);
}

// Computes the row coverage of the given solution by including the best non-redundant columns.
inline void compute_reduced_row_coverage(Instance const&            inst,
                                         std::vector<real_t> const& reduced_costs,
                                         Sorter&                    sorter,
                                         CoverCounters<>&           row_coverage,
                                         Solution&                  sol) {
    // TODO(acco): consider moving to member.
    row_coverage.reset(inst.rows.size());
    sorter.sort(sol.idxs, [&](cidx_t j) { return reduced_costs[j]; });

    for (cidx_t j : sol.idxs) {
        auto col = inst.cols[j];
        if (!row_coverage.is_redundant_cover(col))
            row_coverage.cover(col);
    }

    // TODO(any): The search trajectory changes, so we need average testing to see which is faster.
    // TODO(any): Note that the use of our custom sorter might change significantly the performance.

    // The other approach that computes coverage, sort redundant and remove one by one.
    // for (CidxAndCost c : lb_sol.idxs)
    //     row_coverage.cover(inst.cols[c.col]);
    //
    // auto red_cols = std::vector<CidxAndCost>();
    // for (cidx_t s = 0; s < lb_sol.idxs.size(); ++s)
    //    if (row_coverage.is_redundant_uncover(inst.cols[lb_sol.idxs[s].col]))
    //        red_cols.push_back({s, lb_sol.idxs[s].cost});
    //
    // sorter.sort(red_cols, [&](CidxAndCost c) { return -c.cost; });
    //
    // for (CidxAndCost c : red_cols) {
    //    auto col = inst.cols[lb_sol.idxs[c.col].col];
    //    if (!row_coverage.is_redundant_uncover(col)) {
    //        row_coverage.uncover(col);
    //        lb_sol.idxs[c.col].col = CFT_REMOVED_IDX;
    //    }
    //}
    //
    // remove_if(lb_sol.idxs, [](CidxAndCost c) { return c.col == CFT_REMOVED_IDX; });
}

// Computes the subgradient squared norm according to the given row coverage.
inline real_t compute_subgradient_norm(CoverCounters<> const& row_coverage) {
    int32_t norm = 0;
    for (size_t i = 0; i < row_coverage.size(); ++i) {
        int32_t violation = 1 - row_coverage[i];
        norm += violation * violation;
    }
    return static_cast<real_t>(norm);
}

// Defines lagrangian multipliers as a perturbation of the given ones.
inline void perturb_lagr_multipliers(std::vector<real_t>& lagr_mult, cft::prng_t& rnd) {
    for (float& u : lagr_mult) {
        u *= rnd_real(rnd, 0.9F, 1.1F);
        assert(std::isfinite(u) && "Multiplier is not finite");
    }
}

inline void update_lagr_mult(CoverCounters<> const& row_coverage,
                             real_t                 step_factor,
                             std::vector<real_t>&   lagr_mult) {

    for (size_t i = 0; i < row_coverage.size(); ++i) {
        auto violation = static_cast<real_t>(1 - row_coverage[i]);

        real_t old_mult   = lagr_mult[i];
        real_t delta_mult = step_factor * violation;
        lagr_mult[i]      = cft::max(0.0F, old_mult + delta_mult);
        assert(std::isfinite(lagr_mult[i]) && "Multiplier is not finite");
    }
}

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
        return 0.0;  // the LB is exactly 0 (no negative reduced costs)
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

        auto next_step_size = StepSizeManager(20, step_size);
        auto should_exit    = ExitConditionManager(300);
        auto should_price   = PricingManager(10, std::min(1000UL, nrows / 3));
        auto best_core_lb   = _reset_mult_and_lb(core.inst.costs, best_lagr_mult);
        auto best_real_lb   = limits<real_t>::min();

        size_t max_iters = 10 * nrows;
        for (size_t iter = 0; iter < max_iters && best_real_lb < max_real_lb; ++iter) {

            update_lbsol_and_reduced_costs(core.inst, lagr_mult, lb_sol, reduced_costs);
            compute_reduced_row_coverage(core.inst, reduced_costs, sorter, row_coverage, lb_sol);
            real_t norm = compute_subgradient_norm(row_coverage);

            if (lb_sol.cost > best_core_lb) {
                IF_DEBUG(fmt::print("SUBG > New best lower bound: {}\n", lb_sol.cost));
                best_core_lb   = lb_sol.cost;
                best_lagr_mult = lagr_mult;
            }

            if (norm == 0.0) {
                assert(best_core_lb < cutoff && "Optimum is above cutoff");
                assert(best_core_lb == lb_sol.cost && "Inconsistent lower bound");
                fmt::print("SUBG > Found optimal solution.\n");
                best_lagr_mult = lagr_mult;
                return best_core_lb;
            }

            if (should_exit(iter, best_core_lb))
                return best_core_lb;

            step_size          = next_step_size(iter, lb_sol.cost);
            real_t step_factor = step_size * (best_ub - lb_sol.cost) / norm;
            update_lagr_mult(row_coverage, step_factor, lagr_mult);

            if (should_price(iter) && iter < max_iters - 1) {
                real_t real_lb = price(orig_inst, lagr_mult, core);
                should_price.update(best_core_lb, real_lb, best_ub);

                fmt::print("SUBG > {:4}: Pricing: core LB: {}, real LB: {}\n",
                           iter,
                           best_core_lb,
                           real_lb);

                best_real_lb = max(best_real_lb, real_lb);
                best_core_lb = _reset_mult_and_lb(core.inst.costs, lagr_mult);
            }
        }
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

            if (norm == 0.0) {  // Return optimum
                assert(best_core_lb < cutoff && "Optimum is above cutoff");
                assert(best_core_lb == lb_sol.cost && "Inconsistent lower bound");
                fmt::print("HEUR > Found optimal solution.\n");
                best_lagr_mult = lagr_mult;
                best_sol       = lb_sol;
                return;
            }

            greedy_sol.idxs.clear();
            greedy(inst, lagr_mult, reduced_costs, greedy_sol, cutoff);
            if (greedy_sol.cost <= cutoff - CFT_EPSILON) {
                cutoff   = greedy_sol.cost;
                best_sol = greedy_sol;
                fmt::print("HEUR > Improved current solution {}\n", best_sol.cost);
                IF_DEBUG(check_solution(inst, best_sol));
            }

            real_t step_factor = step_size * (best_sol.cost - lb_sol.cost) / norm;
            update_lagr_mult(row_coverage, step_factor, lagr_mult);
        }
    }
};


}  // namespace cft

#endif