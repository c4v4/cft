// Copyright (c) 2024 Francesco Cavaliere
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

#ifndef CFT_SRC_SUBGRADIENT_UTILS_HPP
#define CFT_SRC_SUBGRADIENT_UTILS_HPP


#include <cstddef>

#include "core/cft.hpp"
#include "utils/assert.hpp"  // IWYU pragma:  keep
#include "utils/limits.hpp"
#include "utils/utility.hpp"

namespace cft { namespace local { namespace {

    // Step size manager functor.
    class StepSizeManager {
        size_t period;
        size_t next_update_iter;
        real_t curr_step_size;
        real_t min_lower_bound;
        real_t max_lower_bound;

    public:
        StepSizeManager(size_t c_period, real_t c_init_step_size)
            : period(c_period)
            , next_update_iter(c_period)
            , curr_step_size(c_init_step_size)
            , min_lower_bound(limits<real_t>::max())
            , max_lower_bound(limits<real_t>::min()) {
        }

        // Computes the next step size.
        real_t operator()(size_t iter, real_t lower_bound) {
            min_lower_bound = min(min_lower_bound, lower_bound);
            max_lower_bound = max(max_lower_bound, lower_bound);
            if (iter == next_update_iter) {
                next_update_iter += period;
                real_t diff = (max_lower_bound - min_lower_bound) / abs(max_lower_bound);
                assert(diff >= 0.0_F && "Negative difference in lower bounds");
                if (diff > 0.01_F)
                    curr_step_size /= 2.0_F;
                if (diff <= 0.001_F)
                    curr_step_size *= 1.5_F;
                min_lower_bound = limits<real_t>::max();
                max_lower_bound = limits<real_t>::min();
            }
            return curr_step_size;
        }
    };

    class ExitConditionManager {
        size_t period;
        size_t next_update_iter;
        real_t prev_lower_bound;

    public:
        ExitConditionManager(size_t c_period)
            : period(c_period)
            , next_update_iter(c_period)
            , prev_lower_bound(limits<real_t>::min()) {
        }

        // Evaluates the exit condition by comparing the current best lower-bound with the
        // previous period's best lower-bound.
        bool operator()(Environment const& env, size_t iter, real_t lower_bound) {
            if (iter == next_update_iter) {
                next_update_iter += period;
                real_t abs_improvement = lower_bound - prev_lower_bound;
                real_t rel_improvement = abs_improvement / lower_bound;
                prev_lower_bound       = lower_bound;
                return abs_improvement < env.abs_subgrad_exit &&
                       rel_improvement < env.rel_subgrad_exit;
            }
            return false;
        }
    };

    // Functional object managing the pricing frequency.
    class PricingManager {
        size_t period;
        size_t next_update_iter;
        size_t max_period_increment;

    public:
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
            if (delta <= 1e-6_F)
                period = min(max_period_increment, 10 * period);
            else if (delta <= 0.02_F)
                period = min(max_period_increment, 5 * period);
            else if (delta <= 0.2_F)
                period = min(max_period_increment, 2 * period);
            else
                period = 10;

            next_update_iter += period;
        }
    };

}  // namespace
}  // namespace local
}  // namespace cft


#endif /* CFT_SRC_SUBGRADIENT_UTILS_HPP */
