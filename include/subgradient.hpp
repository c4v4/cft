#ifndef CFT_INCLUDE_SUBGRADIENT_HPP
#define CFT_INCLUDE_SUBGRADIENT_HPP

#include <algorithm>
#include <vector>

#include "Instance.hpp"
#include "cft.hpp"
#include "coverage.hpp"
#include "limits.hpp"
#include "util_functions.hpp"

namespace cft {

namespace subgradient {
    // Subgradient result.
    struct Result {
        // List of lagrangian multipliers.
        std::vector<std::vector<real_t>> multipliers;

        // Best lower bound found during the execution.
        real_t lower_bound;
    };
}

namespace {

    // Manages the step size used in the multiplier update formula.
    struct StepSizeManager {

        real_t step;

        size_t period;
        size_t iter;
        real_t min_lower_bound;
        real_t max_lower_bound;

        // Updates the step size according to the current search state and the given lower bound.
        inline void update(real_t lower_bound) {
            min_lower_bound = cft::min(min_lower_bound, lower_bound);
            max_lower_bound = cft::max(max_lower_bound, lower_bound);

            ++iter;
            if (iter == period) {

                real_t const diff = (max_lower_bound - min_lower_bound) / max_lower_bound;
                if (diff > 0.01)
                    step /= 2.0;
                else if (diff <= 0.001)
                    step *= 1.5;

                iter            = 0;
                max_lower_bound = limits<real_t>::min();
                min_lower_bound = limits<real_t>::max();
            }
        }
    };

    StepSizeManager make_step_size_manager(real_t initial_step, size_t period) {
        return StepSizeManager{initial_step,
                               period,
                               /*iter=*/0,
                               /*min_lower_bound=*/limits<real_t>::max(),
                               /*max_lower_bound=*/limits<real_t>::min()};
    }

    // Manages the pricing period.
    struct PricingPeriodManager {
        size_t period;
        size_t max_period;
        size_t iter;

        // Increments the iteration counter and returns whether a pricing should be performed.
        inline bool increment() {
            ++iter;
            assert(iter <= period);
            return iter == period;
        }

        // Resets the counter and updates the period according to the given bounds.
        inline void reset(real_t global_lower_bound,
                          real_t local_lower_bound,
                          real_t global_upper_bound) {
            assert(iter == period);
            iter = 0;

            real_t const delta = (local_lower_bound - global_lower_bound) / global_upper_bound;
            if (delta <= 0.000001)
                period = cft::min(max_period, period * 10);
            else if (delta <= 0.02)
                period = cft::min(max_period, period * 5);
            else if (delta <= 0.2)
                period = cft::min(max_period, period * 2);
            else
                period = 10;
        }
    };

    PricingPeriodManager make_pricing_period_manager(size_t period, size_t max_period) {
        return PricingPeriodManager{period, max_period, /*iter=*/0};
    }

    struct ExitConditionManager {

        size_t period;
        size_t iter;
        real_t reference_lower_bound;

        bool finished(real_t lower_bound) {

            ++iter;
            if (iter == period) {
                iter = 0;

                real_t const improvement = lower_bound - reference_lower_bound;
                // TODO: in the old code we were moltiplying the gap by 2. Why?!
                real_t const gap = improvement / lower_bound;

                reference_lower_bound = lower_bound;

                return improvement < 1.0 && gap < 0.001;
            }

            return false;
        }
    };

    ExitConditionManager make_exit_condition_manager(size_t period) {
        return ExitConditionManager{period,
                                    /*iter=*/0,
                                    /*reference_lower_bound=*/limits<real_t>::min()};
    }

    // A solution, i.e., a set of columns and its associated lower bound.
    struct Solution {
        // The index and reduced cost of columns defining the solution.
        std::vector<std::pair<cidx_t, real_t>> col_info;

        // The lower bound associated with the solution.
        real_t lower_bound;
    };

    // Computes a solution by inspection by including all columns having a negative reduced cost.
    Solution compute_solution(Instance const& inst, std::vector<real_t> const& multipliers) {
        auto solution = Solution{};

        solution.lower_bound = 0;
        for (real_t const value : multipliers)
            solution.lower_bound += value;

        for (size_t j = 0; j < inst.cols.size(); ++j) {

            real_t reduced_cost = inst.costs[j];
            for (ridx_t i : inst.cols[j])
                reduced_cost -= multipliers[i];

            if (reduced_cost < 0) {
                solution.col_info.emplace_back(j, reduced_cost);
                solution.lower_bound += reduced_cost;
            }
        }

        return solution;
    }

    // Computes the row coverage of the given solution.
    CoverCounters<uint16_t> compute_solution_row_coverage(Instance const& inst,
                                                          Solution&       solution,
                                                          bool include_redundant_columns) {
        // TODO: consider moving to member.
        CoverCounters<uint16_t> solution_row_coverage = make_cover_counters(inst.rows.size());

        if (include_redundant_columns)
            for (auto const& c : solution.col_info)
                solution_row_coverage.cover(inst.cols[c.first]);
        else {
            std::sort(solution.col_info.begin(),
                      solution.col_info.end(),
                      [](std::pair<cidx_t, real_t> const& a, std::pair<cidx_t, real_t> const& b) {
                          return a.second < b.second;
                      });
            for (auto const& c : solution.col_info) {
                auto const& col = inst.cols[c.first];
                if (!solution_row_coverage.is_redundant_cover(col))
                    solution_row_coverage.cover(col);
            }
        }

        return solution_row_coverage;
    }

    // Computes the subgradient squared norm according to the rows covered by the given solution.
    uint32_t compute_subgradient_squared_norm(
        Instance const&                inst,
        CoverCounters<uint16_t> const& solution_row_coverage) {

        uint32_t norm = 0.0;

        for (size_t i = 0; i < inst.rows.size(); ++i) {
            uint32_t violation = 1 - solution_row_coverage[i];
            norm += violation * violation;
        }

        return norm;
    }

    // Performs a dual ascent on the given instance.
    // According to the explore flag, the result will be a single near-optimal multipliers, when
    // explore is false, or a list of them, when explore is true.
    subgradient::Result dual_ascent(Instance const&            inst,
                                    real_t                     upper_bound,
                                    std::vector<real_t> const& initial_multipliers,
                                    size_t                     max_iterations,
                                    bool                       explore) {

        auto best = subgradient::Result{};

        auto step_size_manager = make_step_size_manager(/*initial_step=*/0.1, /*period=*/20);

        auto pricing_period_manager = make_pricing_period_manager(
            /*period=*/10,
            /*max_period=*/cft::min(1000ul, inst.rows.size() / 3));
        auto exit_condition_manager = make_exit_condition_manager(/*period=*/300);

        // TODO: use the pricing manager.
        (void)pricing_period_manager;

        real_t best_lower_bound = limits<real_t>::min();

        // TODO: consider moving to members.
        auto multipliers      = initial_multipliers;
        auto best_multipliers = initial_multipliers;

        // TODO: Maybe use subgradient::Result directly.
        auto multipliers_list = std::vector<std::vector<real_t>>();

        for (size_t iter = 0; iter < max_iterations; ++iter) {

            auto solution = compute_solution(inst, multipliers);

            CoverCounters<uint16_t> solution_row_coverage = compute_solution_row_coverage(
                inst,
                solution,
                /*include_redundant_columns=*/explore);

            uint32_t squared_norm = compute_subgradient_squared_norm(inst, solution_row_coverage);

            // The subgradient squared norm can only be 0 when no constraint is violated, i.e., we
            // found a feasible primal solution during a lower bound computation, then the solution
            // is optimal.
            if (squared_norm == 0) {
                // Optimal solution for the (sub) instance ?!
                // TODO: store u / solution and return?
            }

            fmt::print("Lower bound = {}\n", solution.lower_bound);

            if (solution.lower_bound > best_lower_bound) {
                fmt::print("                                                Lower bound = {}\n",
                           solution.lower_bound);
                best_lower_bound = solution.lower_bound;
                best_multipliers = multipliers;
            }

            if (explore) {
                multipliers_list.emplace_back(multipliers);
            } else {
                if (exit_condition_manager.finished(best_lower_bound))
                    break;

                bool const should_price = pricing_period_manager.increment();
                if (should_price) {
                    pricing_period_manager.reset(/*TODO: global_lower_bound*/ solution.lower_bound,
                                                 solution.lower_bound,
                                                 upper_bound);

                    // TODO: pricing is missing!
                }
            }

            step_size_manager.update(solution.lower_bound);

            // Update the multipliers.
            for (size_t i = 0; i < inst.rows.size(); ++i) {

                real_t const normalized_bound_diff = (upper_bound - solution.lower_bound) /
                                                     squared_norm;
                real_t const violation = 1 - solution_row_coverage[i];

                multipliers[i] = cft::max(
                    0.0,
                    multipliers[i] + step_size_manager.step * normalized_bound_diff * violation);
            }
        }

        if (explore)
            return subgradient::Result{multipliers_list, best_lower_bound};
        else
            return subgradient::Result{{best_multipliers}, best_lower_bound};
    }


}  // namespace

namespace subgradient {

    // Greedily creates lagrangian multipliers for the given instance.
    std::vector<real_t> make_greedy_multipliers(Instance const& inst) {
        auto multipliers = std::vector<real_t>(inst.rows.size(), limits<real_t>::max());

        for (size_t i = 0; i < inst.rows.size(); ++i) {
            for (cidx_t const j : inst.rows[i]) {
                real_t const candidate = inst.costs[j] / inst.cols[j].size();
                multipliers[i]         = cft::min(multipliers[i], candidate);
            }
        }

        return multipliers;
    }

    std::vector<real_t> make_perturbed_multipliers(std::vector<real_t> const& multipliers,
                                                   cft::prng_t&               rnd) {
        auto perturbed_multipliers = std::vector<real_t>(multipliers.size());
        auto urd                   = std::uniform_real_distribution<real_t>(0.9, 1.1);

        for (size_t i = 0; i < multipliers.size(); ++i)
            perturbed_multipliers[i] = urd(rnd) * multipliers[i];

        return perturbed_multipliers;
    }

    // Runs the subgradient phase on the given instance.
    // Returns a result containting the best found lower bound and its associated lagrangian
    // multipliers.
    Result optimize(Instance const& inst, real_t upper_bound, std::vector<real_t> const& u0) {
        return dual_ascent(inst,
                           upper_bound,
                           u0,
                           /*max_iterations=*/10 * inst.rows.size(),
                           /*explore=*/false);
    }

    // Runs the subgradient phase on the given instance to generate a list of near-optimal
    // lagrangian multipliers.
    // Returns a result containing the above-mentioned list and the best found lower bound.
    Result explore(Instance const& inst, real_t upper_bound, std::vector<real_t> const& u0) {
        return dual_ascent(inst,
                           upper_bound,
                           u0,
                           /*max_iterations=*/250,
                           /*explore=*/true);
    }

}  // namespace subgradient

}  // namespace cft

#endif