#ifndef CFT_INCLUDE_SUBGRADIENT_HPP
#define CFT_INCLUDE_SUBGRADIENT_HPP

#include <cassert>
#include <cmath>
#include <cstddef>
#include <vector>

#include "core/cft.hpp"
#include "core/coverage.hpp"
#include "core/limits.hpp"
#include "core/random.hpp"
#include "core/utility.hpp"
#include "fmt/base.h"
#include "greedy/Greedy.hpp"
#include "instance/Instance.hpp"
#include "subgradient/Pricer.hpp"

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

// A solution, i.e., a set of columns and its associated lower bound.
struct SubgradientSolution {
    // The index and reduced cost of columns defining the solution.
    std::vector<CidxAndCost> col_info;
    // The lower bound associated with the solution.
    real_t lower_bound;
};

// Computes a solution by inspection by including all columns having a negative reduced cost.
inline SubgradientSolution compute_subgradient_solution(Instance const&            inst,
                                                        std::vector<real_t> const& lagr_mult) {
    auto sol = SubgradientSolution{};

    sol.lower_bound = 0;
    for (real_t const value : lagr_mult)
        sol.lower_bound += value;

    for (cidx_t j = 0; j < inst.cols.size(); ++j) {

        real_t reduced_cost = inst.costs[j];
        for (ridx_t i : inst.cols[j])
            reduced_cost -= lagr_mult[i];

        if (reduced_cost < 0.0) {
            sol.col_info.push_back({j, reduced_cost});
            sol.lower_bound += reduced_cost;
        }
    }

    return sol;
}

// Computes the row coverage of the given solution.
inline CoverCounters<> compute_row_coverage(Instance const& inst, SubgradientSolution const& sol) {
    // TODO(acco): consider moving to member.
    auto row_coverage = CoverCounters<>(inst.rows.size());

    for (auto const& c : sol.col_info)
        row_coverage.cover(inst.cols[c.col]);

    return row_coverage;
}

// Computes the row coverage of the given solution by including the best non-redundant columns.
inline CoverCounters<> compute_reduced_row_coverage(Instance const&      inst,
                                                    Sorter&              sorter,
                                                    SubgradientSolution& sol) {
    // TODO(acco): consider moving to member.
    auto row_coverage = CoverCounters<>(inst.rows.size());
    sorter.sort(sol.col_info, [](CidxAndCost a) { return a.cost; });

    // TODO(acco): consider the opposite approach in which we first add all columns, identify the
    // redundant ones, sort them and remove the worst. This may allows us to sort less columns.
    for (auto const& c : sol.col_info) {
        auto const& col = inst.cols[c.col];
        if (!row_coverage.is_redundant_cover(col))
            row_coverage.cover(col);
    }

    return row_coverage;
}

// Computes the subgradient squared norm according to the given row coverage.
inline real_t compute_subgradient_norm(Instance const&                inst,
                                       CoverCounters<uint16_t> const& row_coverage) {
    int32_t norm = 0;
    for (size_t i = 0; i < inst.rows.size(); ++i) {
        int32_t violation = 1 - row_coverage[i];
        norm += violation * violation;
    }
    return static_cast<real_t>(norm);
}

// Greedily creates lagrangian multipliers for the given instance.
inline std::vector<real_t> compute_greedy_multipliers(Instance const& inst) {
    auto lagr_mult = std::vector<real_t>(inst.rows.size(), limits<real_t>::max());

    for (size_t i = 0; i < inst.rows.size(); ++i)
        for (cidx_t const j : inst.rows[i]) {
            real_t const candidate = inst.costs[j] / static_cast<real_t>(inst.cols[j].size());
            lagr_mult[i]           = cft::min(lagr_mult[i], candidate);
        }

    return lagr_mult;
}

// Defines lagrangian multipliers as a perturbation of the given ones.
inline void perturb_lagr_multipliers(std::vector<real_t>& lagr_mult, cft::prng_t& rnd) {
    for (float& u : lagr_mult) {
        u *= rnd_real(rnd, 0.9F, 1.1F);
        assert(std::isfinite(u) && "Multiplier is not finite");
    }
}

// Subgradient phase of the Three-phase algorithm.
// TODO(acco): Consider implementing it as a functor.
inline real_t subgradient(Instance const&      orig_inst,
                       InstAndMap&          core,
                       Sorter&              sorter,
                       real_t               cutoff,
                       real_t               best_ub,
                       real_t&              step_size,
                       std::vector<real_t>& best_lagr_mult) {

    size_t const nrows = orig_inst.rows.size();

    assert(!orig_inst.cols.empty() && "Empty instance");
    assert(!core.inst.cols.empty() && "Empty core instance");
    assert(nrows == core.inst.rows.size() && "Incompatible instances");

    auto next_step_size = StepSizeManager(20, step_size);
    auto should_exit    = ExitConditionManager(300);
    auto should_price   = PricingManager(10, std::min(1000UL, nrows / 3));
    auto price          = Pricer();

    // TODO(acco): consider moving to members.
    auto lagr_mult    = best_lagr_mult;
    auto best_core_lb = limits<real_t>::min();

    size_t max_iters = 10 * nrows;
    for (size_t iter = 0; iter < max_iters; ++iter) {
        auto   sol          = compute_subgradient_solution(core.inst, lagr_mult);
        auto   row_coverage = compute_reduced_row_coverage(core.inst, sorter, sol);
        real_t norm         = compute_subgradient_norm(core.inst, row_coverage);

        if (sol.lower_bound > best_core_lb) {
            IF_DEBUG(fmt::print("SUBG > New best lower bound: {}\n", sol.lower_bound));
            best_core_lb   = sol.lower_bound;
            best_lagr_mult = lagr_mult;
        }

        if (norm == 0.0) {
            assert(best_core_lb < cutoff && "Optimum is above cutoff");
            assert(best_core_lb == sol.lower_bound && "Inconsistent lower bound");
            fmt::print("SUBG > Found optimal solution.\n");
            best_lagr_mult = lagr_mult;
            return best_core_lb;
        }

        if (should_exit(iter, best_core_lb))
            return best_core_lb;

        step_size = next_step_size(iter, sol.lower_bound);
        for (size_t i = 0; i < nrows; ++i) {
            real_t normalized_bound_diff = (best_ub - sol.lower_bound) / norm;
            auto   violation             = static_cast<real_t>(1 - row_coverage[i]);

            real_t delta_mult = step_size * normalized_bound_diff * violation;
            lagr_mult[i]      = cft::max(0.0F, lagr_mult[i] + delta_mult);
            assert(std::isfinite(lagr_mult[i]) && "Multiplier is not finite");
        }

        if (should_price(iter) && iter < max_iters - 1) {
            real_t real_lb = price(orig_inst, lagr_mult, core);
            should_price.update(best_core_lb, real_lb, best_ub);
            fmt::print("SUBG > {:4}: Pricing: core LB: {}, real LB: {}\n",
                       iter,
                       best_core_lb,
                       real_lb);

            if (real_lb >= cutoff - CFT_EPSILON) {
                fmt::print("SUBG > Unpromising set of columns.\n");
                return real_lb;
            }
            best_core_lb = limits<real_t>::min();  // TODO(cava): avoid pricing during last iter
        }
    }
    return best_core_lb;
}

// Heuristic phase of the Three-phase algorithm.
// NOTE: It seems that in the original they store the lagrangian multipliers associated to the best
// lower bound, however, it seems to work better if we store the lagrangian multipliers associated
// to the best greedy solution. (But this might be due to the different column fixing we are using).
// TODO(acco): Consider implementing it as a functor.
inline real_t heuristic(Instance const&      inst,
                      Greedy&              greedy,
                      real_t               cutoff,
                      real_t               step_size,
                      Solution&            best_sol,
                      std::vector<real_t>& best_greedy_lagr_mult) {

    auto   lagr_mult        = best_greedy_lagr_mult;
    auto   greedy_sol       = Solution();
    auto   best_lower_bound = limits<real_t>::min();
    size_t max_iters        = 250;  // TODO(all): consider making it a parameter.
    for (size_t iter = 0; iter < max_iters; ++iter) {
        auto   sol          = compute_subgradient_solution(inst, lagr_mult);
        auto   row_coverage = compute_row_coverage(inst, sol);
        real_t norm         = compute_subgradient_norm(inst, row_coverage);

        if (sol.lower_bound > best_lower_bound)
            best_lower_bound = sol.lower_bound;

        assert(best_lower_bound <= best_sol.cost && "Inconsistent lower bound");
        if (best_lower_bound >= cutoff - CFT_EPSILON) {
            fmt::print("HEUR > Unpromising set of columns.\n");
            return best_lower_bound;
        }

        if (norm == 0.0) {  // Return optimum
            assert(best_lower_bound < cutoff && "Optimum is above cutoff");
            assert(best_lower_bound == sol.lower_bound && "Inconsistent lower bound");
            fmt::print("HEUR > Found optimal solution.\n");
            best_greedy_lagr_mult = lagr_mult;
            best_sol.cost         = sol.lower_bound;
            best_sol.idxs.clear();
            for (auto c : sol.col_info)
                best_sol.idxs.push_back(c.col);
            return best_lower_bound;
        }

        greedy_sol.idxs.clear();
        greedy(inst, lagr_mult, greedy_sol, cutoff);
        if (greedy_sol.cost <= cutoff - CFT_EPSILON) {
            cutoff                = greedy_sol.cost;
            best_sol              = greedy_sol;
            best_greedy_lagr_mult = lagr_mult;
            IF_DEBUG(check_solution(inst, best_sol));
        }

        for (size_t i = 0; i < inst.rows.size(); ++i) {
            real_t normalized_bound_diff = (best_sol.cost - sol.lower_bound) / norm;
            auto   violation             = static_cast<real_t>(1 - row_coverage[i]);

            real_t delta_mult = step_size * normalized_bound_diff * violation;
            lagr_mult[i]      = cft::max(0.0F, lagr_mult[i] + delta_mult);
            assert(std::isfinite(lagr_mult[i]) && "Multiplier is not finite");
        }
    }
    return best_lower_bound;
}

}  // namespace cft

#endif