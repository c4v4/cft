#ifndef CFT_INCLUDE_SUBGRADIENT_HPP
#define CFT_INCLUDE_SUBGRADIENT_HPP

#include <vector>

#include "Instance.hpp"
#include "cft.hpp"
#include "coverage.hpp"
#include "util_functions.hpp"

namespace cft {

// Result of the subgradient optimize procedure.
struct OptimizeResult {
    // Best lower bound found during the procedure.
    real_t lower_bound;
    // Lagrangian multipliers associated with the best found lower bound.
    std::vector<real_t> lagr_mult;
};

inline OptimizeResult make_optimize_result(std::vector<real_t> const& lagr_mult) {
    return OptimizeResult{limits<real_t>::min(), lagr_mult};
}

// Result of the subgradient explore procedure.
struct ExploreResult {
    // Best lower bound found during the procedure.
    real_t lower_bound;
    // List of lagrangian multipliers found during the procedure.
    std::vector<std::vector<real_t>> lagr_mult_list;
};

// Step size manager functor.
struct StepSizeManager {
    size_t period;
    size_t next_update_iter;
    real_t curr_step_size;
    real_t min_lower_bound;
    real_t max_lower_bound;

    // Computes the next step size.
    CFT_NODISCARD real_t operator()(size_t iter, real_t lower_bound) {
        min_lower_bound = cft::min(min_lower_bound, lower_bound);
        max_lower_bound = cft::max(max_lower_bound, lower_bound);
        if (iter == next_update_iter) {
            real_t const diff = (max_lower_bound - min_lower_bound) / max_lower_bound;
            if (diff > 0.01)
                curr_step_size /= 2.0;
            else if (diff <= 0.001)
                curr_step_size *= 1.5;
            next_update_iter += period;
            min_lower_bound = limits<real_t>::max();
            max_lower_bound = limits<real_t>::min();
        }
        return curr_step_size;
    }
};

inline StepSizeManager make_step_size_manager(size_t period, real_t init_step_size) {
    return StepSizeManager{period,
                           period,
                           init_step_size,
                           limits<real_t>::max(),
                           limits<real_t>::min()};
}

// A solution, i.e., a set of columns and its associated lower bound.
struct Solution {
    // The index and reduced cost of columns defining the solution.
    std::vector<std::pair<cidx_t, real_t>> col_info;

    // The lower bound associated with the solution.
    real_t lower_bound;
};

// Computes a solution by inspection by including all columns having a negative reduced cost.
inline Solution compute_solution(Instance const& inst, std::vector<real_t> const& lagr_mult) {
    auto sol = Solution{};

    sol.lower_bound = 0;
    for (real_t const value : lagr_mult)
        sol.lower_bound += value;

    for (size_t j = 0; j < inst.cols.size(); ++j) {

        real_t reduced_cost = inst.costs[j];
        for (ridx_t i : inst.cols[j])
            reduced_cost -= lagr_mult[i];

        if (reduced_cost < 0.0) {
            sol.col_info.emplace_back(j, reduced_cost);
            sol.lower_bound += reduced_cost;
        }
    }

    return sol;
}

// Computes the row coverage of the given solution.
inline CoverCounters<uint16_t> compute_row_coverage(Instance const& inst, Solution& sol) {
    // TODO: consider moving to member.
    CoverCounters<uint16_t> row_coverage = make_cover_counters(inst.rows.size());

    for (auto const& c : sol.col_info)
        row_coverage.cover(inst.cols[c.first]);

    return row_coverage;
}

// Computes the subgradient squared norm according to the given row coverage.
inline uint32_t compute_subgradient_norm(Instance const&                inst,
                                         CoverCounters<uint16_t> const& row_coverage) {
    uint32_t norm = 0.0;
    for (size_t i = 0; i < inst.rows.size(); ++i) {
        uint32_t violation = 1 - row_coverage[i];
        norm += violation * violation;
    }
    return norm;
}

// Greedily creates lagrangian multipliers for the given instance.
std::vector<real_t> compute_greedy_multipliers(Instance const& inst) {
    auto lagr_mult = std::vector<real_t>(inst.rows.size(), limits<real_t>::max());

    for (size_t i = 0; i < inst.rows.size(); ++i) {
        for (cidx_t const j : inst.rows[i]) {
            real_t const candidate = inst.costs[j] / inst.cols[j].size();
            lagr_mult[i]           = cft::min(lagr_mult[i], candidate);
        }
    }

    return lagr_mult;
}

// TODO: Consider implementing it as a functor.
OptimizeResult optimize(Instance const&            inst,
                        real_t                     upper_bound,
                        std::vector<real_t> const& initial_lagr_mult) {

    auto next_step_size = make_step_size_manager(20, 0.1);

    // TODO: consider moving to members.
    auto lagr_mult = initial_lagr_mult;
    auto best      = make_optimize_result(initial_lagr_mult);

    size_t max_iters = 10 * inst.rows.size();
    for (size_t iter = 0; iter < max_iters; ++iter) {
        auto     sol          = compute_solution(inst, lagr_mult);
        auto     row_coverage = compute_row_coverage(inst, sol);
        uint32_t norm         = compute_subgradient_norm(inst, row_coverage);

        if (norm == 0) {
            // No constraints are violated.
            // Optimal (sub) instance solution?
        }

        if (sol.lower_bound > best.lower_bound) {
            fmt::print("Lower bound = {}\n", sol.lower_bound);
            best.lower_bound = sol.lower_bound;
            best.lagr_mult   = lagr_mult;
        }

        // TODO: add pricing.

        real_t step_size = next_step_size(iter, sol.lower_bound);

        for (size_t i = 0; i < inst.rows.size(); ++i) {
            real_t normalized_bound_diff = (upper_bound - sol.lower_bound) / norm;
            real_t violation             = 1 - row_coverage[i];

            real_t delta_mult = step_size * normalized_bound_diff * violation;
            lagr_mult[i]      = cft::max(0.0, lagr_mult[i] + delta_mult);
        }
    }

    return best;
}

// TODO: Consider implementing it as a functor.
ExploreResult explore() {
    // TODO: implement me!
    return ExploreResult{};
}

}  // namespace cft

#endif