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
#include "core/sort.hpp"
#include "greedy/Enumerator.hpp"
#include "greedy/RedundancySet.hpp"
#include "instance/Instance.hpp"

namespace cft {

inline real_t init_redund_set(RedundancyData&            red_data,
                              Instance const&            inst,
                              std::vector<cidx_t> const& sol,
                              real_t                     ub,
                              Sorter&                    sorter) {

    red_data.cols_to_remove.clear();
    red_data.total_cover.reset(inst.rows.size());
    red_data.ub = ub;

    real_t lb = 0.0;
    for (cidx_t j : sol)
        red_data.total_cover.cover(inst.cols[j]);
    for (cidx_t j : sol)
        if (red_data.total_cover.is_redundant_uncover(inst.cols[j]))
            red_data.redund_set.push_back({j, inst.costs[j]});
        else {
            lb += inst.costs[j];
            if (lb >= ub)
                return lb;
        }

    sorter.sort(red_data.redund_set, [&](CidxAndCost x) { return inst.costs[x.col]; });
    return lb;
}

inline real_t heuristic_removal(RedundancyData& red_set, Instance const& inst, real_t lb) {
    while (lb < red_set.ub && red_set.redund_set.size() > ENUM_VARS) {
        cidx_t j = red_set.redund_set.back().col;
        red_set.redund_set.pop_back();
        red_set.total_cover.uncover(inst.cols[j]);
        red_set.cols_to_remove.push_back(j);

        // They say they update the redudant set after every removal, which has the only effect
        // of exting earlier (with more elements) and start the enumeration.
        // TODO(cava): test if it actually improve/degrade performace/quality
        remove_if(red_set.redund_set, [&](CidxAndCost x) {
            if (red_set.total_cover.is_redundant_uncover(inst.cols[x.col]))
                return false;
            lb += inst.costs[x.col];
            return true;
        });
    }
    return lb;
}

inline real_t enumeration_removal(RedundancyData& red_set, Instance const& inst, real_t lb) {
    assert(red_set.redund_set.size() <= ENUM_VARS);
    real_t old_ub = red_set.ub;
    if (lb >= old_ub || red_set.redund_set.empty())
        return lb;

    // TODO(cava): Redundant set can be an instance like, with a subset of rows and columns
    auto curr_cover                 = make_cover_bits(inst.rows.size());
    bool curr_keep_state[ENUM_VARS] = {};
    bool cols_to_keep[ENUM_VARS]    = {};

    Enumerator<0>::invoke(inst, red_set, lb, curr_keep_state, cols_to_keep);

    if (red_set.ub < old_ub)  // cols_to_keep is updated only if the upper bound is improved
        for (cidx_t r = 0; r < red_set.redund_set.size(); ++r)
            if (!cols_to_keep[r])
                red_set.cols_to_remove.push_back(red_set.redund_set[r].col);

    return red_set.ub;
}

}  // namespace cft

#endif /* CFT_SRC_GREEDY_REDUNDANCY_HPP */
