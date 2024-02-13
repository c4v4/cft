#ifndef CFT_INCLUDE_SUBGRADIENT_HPP
#define CFT_INCLUDE_SUBGRADIENT_HPP

#include <algorithm>  // TODO: remove when replaced min/max
#include <vector>

#include "Instance.hpp"
#include "cft.hpp"
#include "limits.hpp"

namespace cft {
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
            // TODO: replace std::min/max with cav_lib::min/max
            min_lower_bound = std::min(min_lower_bound, lower_bound);
            max_lower_bound = std::max(max_lower_bound, lower_bound);

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
                period = std::min(max_period, period * 10);
            else if (delta <= 0.02)
                period = std::min(max_period, period * 5);
            else if (delta <= 0.2)
                period = std::min(max_period, period * 2);
            else
                period = 10;
        }
    };

    PricingPeriodManager make_pricing_period_manager(size_t period, size_t max_period) {
        return PricingPeriodManager{period, max_period, /*iter=*/0};
    }

}

// Subgradient functor.
struct Subgradient {

    // Subgradient result.
    struct Result {
        // List of lagrangian multipliers.
        std::vector<std::vector<real_t>> u_list;

        // Best lower bound found during the execution.
        real_t lower_bound;
    };

    // Working lagrangian multipliers.
    std::vector<real_t> u;

    // Best lagrangian multipliers.
    std::vector<real_t> u_star;

    // Runs the subgradient phase on the given instance.
    // Returns the best found lower bound found and its associated lagrangian multipliers.
    Result optimize(Instance const& inst, real_t upper_bound, std::vector<real_t> u0) {

        real_t lower_bound = limits<real_t>::min();

        u      = u0;
        u_star = u0;


        return Result{};
    }

    // Result explore() {

    //     return Result{};
    // }
};

};

#endif