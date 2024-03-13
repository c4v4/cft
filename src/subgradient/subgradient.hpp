#ifndef CFT_INCLUDE_SUBGRADIENT_HPP
#define CFT_INCLUDE_SUBGRADIENT_HPP

#include <random>
#include <vector>

#include "core/cft.hpp"
#include "core/coverage.hpp"
#include "core/utility.hpp"
#include "instance/Instance.hpp"

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

inline ExploreResult make_explore_result() {
    return ExploreResult{limits<real_t>::min(), {}};
}

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

inline StepSizeManager make_step_size_manager(size_t period, real_t init_step_size) {
    return StepSizeManager{period,
                           period,
                           init_step_size,
                           limits<real_t>::max(),
                           limits<real_t>::min()};
}

struct ExitConditionManager {
    size_t period;
    size_t next_update_iter;
    real_t reference_lower_bound;

    CFT_NODISCARD bool operator()(size_t iter, real_t lower_bound) {
        if (iter == next_update_iter) {
            next_update_iter += period;
            real_t improvement = lower_bound - reference_lower_bound;
            real_t gap         = improvement / lower_bound;
            return improvement < 1.0 && gap < 0.001;
        }
        return false;
    }
};

inline ExitConditionManager make_exit_condition_manager(size_t period) {
    return ExitConditionManager{period, period, limits<real_t>::min()};
}

// A solution, i.e., a set of columns and its associated lower bound.
struct SubgradientSolution {

    // Info describing a column in solution.
    struct ColInfo {
        // Column index.
        cidx_t idx;
        // Column reduced cost.
        real_t reduced_cost;
    };

    // The index and reduced cost of columns defining the solution.
    std::vector<ColInfo> col_info;

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
            sol.col_info.push_back(SubgradientSolution::ColInfo{j, reduced_cost});
            sol.lower_bound += reduced_cost;
        }
    }

    return sol;
}

// Computes the row coverage of the given solution.
inline CoverCounters<uint16_t> compute_row_coverage(Instance const&            inst,
                                                    SubgradientSolution const& sol) {
    // TODO: consider moving to member.
    auto row_coverage = make_cover_counters(inst.rows.size());

    for (auto const& c : sol.col_info)
        row_coverage.cover(inst.cols[c.idx]);

    return row_coverage;
}

// Computes the row coverage of the given solution by including the best non-redundant columns.
inline CoverCounters<uint16_t> compute_reduced_row_coverage(Instance const&      inst,
                                                            SubgradientSolution& sol) {
    // TODO: consider moving to member.
    CoverCounters<uint16_t> row_coverage = make_cover_counters(inst.rows.size());

    std::sort(sol.col_info.begin(),
              sol.col_info.end(),
              [](SubgradientSolution::ColInfo a, SubgradientSolution::ColInfo b) {
                  return a.reduced_cost < b.reduced_cost;
              });

    // TODO: consider the opposite approach in which we first add all columns, identify the
    // redundant ones, sort them and remove the worst. This may allows us to sort less columns.
    for (auto const& c : sol.col_info) {
        auto const& col = inst.cols[c.idx];
        if (!row_coverage.is_redundant_cover(col))
            row_coverage.cover(col);
    }

    return row_coverage;
}

// Computes the subgradient squared norm according to the given row coverage.
inline real_t compute_subgradient_norm(Instance const&                inst,
                                       CoverCounters<uint16_t> const& row_coverage) {
    uint32_t norm = 0;
    for (size_t i = 0; i < inst.rows.size(); ++i) {
        uint32_t violation = 1 - row_coverage[i];
        norm += violation * violation;
    }
    return norm;
}

// Greedily creates lagrangian multipliers for the given instance.
inline std::vector<real_t> compute_greedy_multipliers(Instance const& inst) {
    auto lagr_mult = std::vector<real_t>(inst.rows.size(), limits<real_t>::max());

    for (size_t i = 0; i < inst.rows.size(); ++i) {
        for (cidx_t const j : inst.rows[i]) {
            real_t const candidate = inst.costs[j] / inst.cols[j].size();
            lagr_mult[i]           = cft::min(lagr_mult[i], candidate);
        }
    }

    return lagr_mult;
}

// Defines lagrangian multipliers as a perturbation of the given ones.
inline std::vector<real_t> compute_perturbed_multipliers(std::vector<real_t> const& multipliers,
                                                         cft::prng_t&               rnd) {
    auto perturbed_lagr_mult = std::vector<real_t>(multipliers.size());
    auto urd                 = std::uniform_real_distribution<real_t>(0.9, 1.1);

    for (size_t i = 0; i < multipliers.size(); ++i)
        perturbed_lagr_mult[i] = urd(rnd) * multipliers[i];

    return perturbed_lagr_mult;
}

// TODO: Consider implementing it as a functor.
inline OptimizeResult optimize(Instance const&            inst,
                               real_t                     upper_bound,
                               std::vector<real_t> const& initial_lagr_mult) {

    auto next_step_size = make_step_size_manager(20, 0.1);
    auto should_exit    = make_exit_condition_manager(300);

    // TODO: consider moving to members.
    auto lagr_mult = initial_lagr_mult;
    auto best      = make_optimize_result(initial_lagr_mult);

    size_t max_iters = 10 * inst.rows.size();
    for (size_t iter = 0; iter < max_iters; ++iter) {
        auto   sol          = compute_subgradient_solution(inst, lagr_mult);
        auto   row_coverage = compute_row_coverage(inst, sol);
        real_t norm         = compute_subgradient_norm(inst, row_coverage);

        // No constraints violated. Optimal (sub) instance solution?
        if (norm == 0) {
        }

        if (sol.lower_bound > best.lower_bound) {
            best.lower_bound = sol.lower_bound;
            best.lagr_mult   = lagr_mult;
        }

        if (should_exit(iter, best.lower_bound))
            break;

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
inline ExploreResult explore(Instance const&            inst,
                             real_t                     upper_bound,
                             std::vector<real_t> const& initial_lagr_mult) {

    auto lagr_mult = initial_lagr_mult;
    auto res       = make_explore_result();

    size_t max_iters = 250;
    for (size_t iter = 0; iter < max_iters; ++iter) {
        auto   sol          = compute_subgradient_solution(inst, lagr_mult);
        auto   row_coverage = compute_reduced_row_coverage(inst, sol);
        real_t norm         = compute_subgradient_norm(inst, row_coverage);

        // No constraints violated. Optimal (sub) instance solution?
        if (norm == 0) {
        }

        if (sol.lower_bound > res.lower_bound)
            res.lower_bound = sol.lower_bound;

        res.lagr_mult_list.push_back(lagr_mult);

        real_t step_size = 0.1;  // TODO: should we vary this?
        for (size_t i = 0; i < inst.rows.size(); ++i) {
            real_t normalized_bound_diff = (upper_bound - sol.lower_bound) / norm;
            real_t violation             = 1 - row_coverage[i];

            real_t delta_mult = step_size * normalized_bound_diff * violation;
            lagr_mult[i]      = cft::max(0.0, lagr_mult[i] + delta_mult);
        }
    }
    return res;
}

}  // namespace cft

#endif