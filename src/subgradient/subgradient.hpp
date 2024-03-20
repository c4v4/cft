#ifndef CFT_INCLUDE_SUBGRADIENT_HPP
#define CFT_INCLUDE_SUBGRADIENT_HPP

#include <cstddef>
#include <random>
#include <vector>

#include "core/cft.hpp"
#include "core/coverage.hpp"
#include "core/limits.hpp"
#include "core/utility.hpp"
#include "instance/Instance.hpp"
#include "subgradient/Pricer.hpp"

namespace cft {

// Result of the subgradient optimize procedure.
struct OptimizeResult {
    // Best lower bound found during the procedure.
    real_t lower_bound = limits<real_t>::min();
    // Lagrangian multipliers associated with the best found lower bound.
    std::vector<real_t> lagr_mult;
};

inline OptimizeResult compute_initial_result(std::vector<real_t> const& lagr_mult) {
    auto res        = OptimizeResult();
    res.lagr_mult   = lagr_mult;
    res.lower_bound = limits<real_t>::min();  // TODO(any): compute the correct LB
    return res;
}

// Result of the subgradient explore procedure.
struct ExploreResult {
    // Best lower bound found during the procedure.
    real_t lower_bound = limits<real_t>::min();
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
    real_t lb_before_pricing;

    PricingManager(size_t c_period, size_t c_max_period_increment)
        : period(c_period)
        , next_update_iter(c_period)
        , max_period_increment(c_max_period_increment)
        , lb_before_pricing(limits<real_t>::min()) {
    }

    CFT_NODISCARD bool operator()(size_t iter, real_t lower_bound, real_t upper_bound) {
        if (iter == next_update_iter + 1)
            _update(lower_bound, upper_bound);

        assert(iter <= next_update_iter);

        if (iter == next_update_iter) {
            lb_before_pricing = lower_bound;
            return true;
        }
        return false;
    }

private:
    void _update(real_t lb_after_pricing, real_t ub) {
        real_t const delta = (lb_after_pricing - lb_before_pricing) / ub;

        size_t next_period = 0;
        if (delta <= 1e-6)
            next_period = std::min(max_period_increment, 10 * period);
        else if (delta <= 0.02)
            next_period = std::min(max_period_increment, 5 * period);
        else if (delta <= 0.2)
            next_period = std::min(max_period_increment, 2 * period);
        else
            next_period = 10;

        next_update_iter += next_period;
        period = next_period;
    }
};

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
    // TODO(acco): consider moving to member.
    auto row_coverage = CoverCounters<>(inst.rows.size());

    for (auto const& c : sol.col_info)
        row_coverage.cover(inst.cols[c.idx]);

    return row_coverage;
}

// Computes the row coverage of the given solution by including the best non-redundant columns.
inline CoverCounters<uint16_t> compute_reduced_row_coverage(Instance const&      inst,
                                                            SubgradientSolution& sol) {
    // TODO(acco): consider moving to member.
    CoverCounters<uint16_t> row_coverage = CoverCounters<>(inst.rows.size());

    std::sort(sol.col_info.begin(),
              sol.col_info.end(),
              [](SubgradientSolution::ColInfo a, SubgradientSolution::ColInfo b) {
                  return a.reduced_cost < b.reduced_cost;
              });

    // TODO(acco): consider the opposite approach in which we first add all columns, identify the
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

    for (size_t i = 0; i < inst.rows.size(); ++i)
        for (cidx_t const j : inst.rows[i]) {
            real_t const candidate = inst.costs[j] / inst.cols[j].size();
            lagr_mult[i]           = cft::min(lagr_mult[i], candidate);
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

// TODO(acco): Consider implementing it as a functor.
inline OptimizeResult optimize(Instance const&            orig_inst,
                               Instance&                  core_inst,
                               real_t                     upper_bound,
                               std::vector<real_t> const& initial_lagr_mult) {

    assert(!orig_inst.cols.empty());
    assert(!core_inst.cols.empty());

    size_t const nrows = orig_inst.rows.size();
    assert(nrows == core_inst.rows.size());

    auto next_step_size = StepSizeManager(20, 0.1);
    auto should_exit    = ExitConditionManager(300);
    auto should_price   = PricingManager(10, std::min(1000UL, nrows / 3));

    // TODO(acco): consider moving to members.
    auto lagr_mult = initial_lagr_mult;
    auto best      = compute_initial_result(initial_lagr_mult);
    auto price     = Pricer();

    size_t max_iters = 10 * nrows;
    for (size_t iter = 0; iter < max_iters; ++iter) {
        auto   sol          = compute_subgradient_solution(core_inst, lagr_mult);
        auto   row_coverage = compute_row_coverage(core_inst, sol);
        real_t norm         = compute_subgradient_norm(core_inst, row_coverage);

        // No constraints violated. Optimal (sub) instance solution?
        if (norm == 0) {
            // TODO(acco): consider updating the upper_bound and storing the solution.
        }

        if (sol.lower_bound > best.lower_bound) {
            best.lower_bound = sol.lower_bound;
            best.lagr_mult   = lagr_mult;
        }

        if (should_exit(iter, best.lower_bound))
            break;

        real_t step_size = next_step_size(iter, sol.lower_bound);
        for (size_t i = 0; i < nrows; ++i) {
            real_t normalized_bound_diff = (upper_bound - sol.lower_bound) / norm;
            real_t violation             = 1 - row_coverage[i];

            real_t delta_mult = step_size * normalized_bound_diff * violation;
            lagr_mult[i]      = cft::max(0.0, lagr_mult[i] + delta_mult);
            assert(std::isfinite(lagr_mult[i]) && "Multiplier is not finite");
        }

        if (should_price(iter, sol.lower_bound, upper_bound))
            price(orig_inst, lagr_mult, core_inst);
    }

    return best;
}

// TODO(acco): Consider implementing it as a functor.
inline ExploreResult explore(Instance const&            inst,
                             real_t                     upper_bound,
                             std::vector<real_t> const& initial_lagr_mult) {

    auto lagr_mult = initial_lagr_mult;
    auto res       = ExploreResult();

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

        real_t step_size = 0.1;  // TODO(acco): should we vary this?
        for (size_t i = 0; i < inst.rows.size(); ++i) {
            real_t normalized_bound_diff = (upper_bound - sol.lower_bound) / norm;
            real_t violation             = 1 - row_coverage[i];

            real_t delta_mult = step_size * normalized_bound_diff * violation;
            lagr_mult[i]      = cft::max(0.0, lagr_mult[i] + delta_mult);
            assert(std::isfinite(lagr_mult[i]) && "Multiplier is not finite");
        }
    }
    return res;
}

}  // namespace cft

#endif