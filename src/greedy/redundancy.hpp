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

#ifndef CFT_SRC_GREEDY_REDUNDANCY_HPP
#define CFT_SRC_GREEDY_REDUNDANCY_HPP

#include "core/cft.hpp"
#include "greedy/Enumerator.hpp"
#include "greedy/RedundancySet.hpp"
#include "instance/Instance.hpp"

namespace cft {

inline void heuristic_removal(RedundancyData& red_set, Instance const& inst) {
    while (red_set.partial_cost < red_set.best_cost && red_set.redund_set.size() > CFT_ENUM_VARS) {
        cidx_t j = red_set.redund_set.back().col;
        red_set.redund_set.pop_back();
        auto uncovered = red_set.total_cover.uncover(inst.cols[j]);
        assert(uncovered == 0);
        red_set.cols_to_remove.push_back(j);

        // They say they update the redudant set after every removal, which has the only effect
        // of exting earlier (with more elements) and start the enumeration.
        // TODO(cava): test if it actually improve/degrade performace/quality
        remove_if(red_set.redund_set, [&](CidxAndCost x) {
            if (red_set.total_cover.is_redundant_uncover(inst.cols[x.col]))
                return false;
            red_set.partial_cost += inst.costs[x.col];
            red_set.partial_cov_count += red_set.partial_cover.cover(inst.cols[x.col]);
            return true;
        });
    }
}

inline void enumeration_removal(RedundancyData& red_set, Instance const& inst) {
    assert(red_set.redund_set.size() <= CFT_ENUM_VARS);
    real_t old_ub = red_set.best_cost;
    if (red_set.partial_cost >= old_ub || red_set.redund_set.empty())
        return;

    // TODO(cava): Redundant set can be an instance like, with a subset of rows and columns
    bool curr_keep_state[CFT_ENUM_VARS] = {};
    bool cols_to_keep[CFT_ENUM_VARS]    = {};

    Enumerator<0>::invoke(inst, red_set, curr_keep_state, cols_to_keep);

    if (red_set.best_cost < old_ub)
        // cols_to_keep is updated only if the upper bound is improved
        for (cidx_t r = 0; r < red_set.redund_set.size(); ++r)
            if (!cols_to_keep[r])
                red_set.cols_to_remove.push_back(red_set.redund_set[r].col);
}

}  // namespace cft

#endif /* CFT_SRC_GREEDY_REDUNDANCY_HPP */
