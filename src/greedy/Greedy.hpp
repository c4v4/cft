// Copyright (c) 2024 Francesco Cavaliere
// This program is free software: you can redistribute it and/or modify it under the terms of the
// GNU General Public License as published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version. This program is distributed in the hope that it
// will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details. You should
// have received a copy of the GNU General Public License along with this program. If not, see
// <https://www.gnu.org/licenses/>.

#ifndef CFT_INCLUDE_GREEDY_HPP
#define CFT_INCLUDE_GREEDY_HPP

#include "Scores.hpp"
#include "core/cft.hpp"
#include "core/coverage.hpp"
#include "core/limits.hpp"
#include "core/sort.hpp"
#include "greedy/redundancy.hpp"
#include "instance/Instance.hpp"

namespace cft {

// This is the greedy step of the 3-phase of the CFT algorithm. It uses a set of Lagrangian
// multipliers to attempt to find an decent solution for the SCP problem (quantity over quality).
// For efficiency, we use a function object instead of a simple function. This allows us to cache
// data structures used in the Greedy step, preventing repeated reallocations. It also keeps
// internal details hidden, unlike using an external struct for cache storage. At the end of the day
// though, it can be mentally thought of as a function.
struct Greedy {
    // Caches
    Sorter         sorter;
    Scores         scores;
    RedundancyData red_set;

    // The greedy algorithm:
    // 1. Initialize column scores (based on the current lagragian multipliers)
    // 2. Add the column with the best score (until the solution is "complete")
    // 3. If present, remove redundant columns from the solution
    // NOTE: a valid solution is returned only if its cost is below the cutoff_cost
    real_t operator()(Instance const&            inst,
                      std::vector<real_t> const& lagr_mult,
                      std::vector<cidx_t>&       sol,
                      real_t                     cutoff_cost  = limits<real_t>::max(),
                      cidx_t                     max_sol_size = limits<cidx_t>::max()) {

        ridx_t nrows_to_cover = inst.rows.size();

        auto& total_cover = red_set.total_cover;
        total_cover.reset(inst.rows.size());

        if (sol.empty())
            scores.init_scores(inst, lagr_mult, sorter);
        else {
            for (cidx_t j : sol)
                nrows_to_cover -= total_cover.cover(inst.cols[j]);
            scores.init_scores(inst, lagr_mult, total_cover, sorter);
        }

        // Fill solution
        while (nrows_to_cover > 0 && sol.size() < max_sol_size) {
            cidx_t jstar = scores.extract_minscore_col(inst, lagr_mult, total_cover, sorter);
            sol.push_back(jstar);
            nrows_to_cover -= total_cover.cover(inst.cols[jstar]);
        }

        // Redundancy removal
        _complete_init_redund_set(red_set, inst, sol, cutoff_cost);
        if (_try_early_exit(red_set, sol))
            return red_set.partial_cost;
        IF_DEBUG(check_redundancy_data(inst, sol, red_set));

        heuristic_removal(red_set, inst);
        if (_try_early_exit(red_set, sol))
            return red_set.partial_cost;
        IF_DEBUG(check_redundancy_data(inst, sol, red_set));

        enumeration_removal(red_set, inst);
        if (red_set.best_cost >= cutoff_cost)
            return red_set.best_cost;

        remove_if(sol, [&](cidx_t j) {
            return any(red_set.cols_to_remove, [j](cidx_t r) { return r == j; });
        });
        return red_set.best_cost;
    }

private:
    void _complete_init_redund_set(RedundancyData&            red_data,
                                   Instance const&            inst,
                                   std::vector<cidx_t> const& sol,
                                   real_t                     cutoff_cost) {

        red_data.redund_set.clear();
        red_data.partial_cover.reset(inst.rows.size());
        red_data.partial_cov_count = 0;
        red_data.cols_to_remove.clear();
        red_data.best_cost    = cutoff_cost;
        red_data.partial_cost = 0.0;

        for (cidx_t j : sol)
            if (red_data.total_cover.is_redundant_uncover(inst.cols[j]))
                red_data.redund_set.push_back({j, inst.costs[j]});
            else {
                red_data.partial_cov_count += red_data.partial_cover.cover(inst.cols[j]);
                red_data.partial_cost += inst.costs[j];
                if (red_data.partial_cost >= cutoff_cost)
                    return;
            }
        sorter.sort(red_data.redund_set, [&](CidxAndCost x) { return inst.costs[x.col]; });
    }

    static bool _try_early_exit(RedundancyData& red_data, std::vector<cidx_t>& sol) {

        if (red_data.partial_cost >= red_data.best_cost || red_data.redund_set.empty())
            return true;  // Discard sol

        if (red_data.partial_cov_count < red_data.partial_cover.size())
            return false;  // Continue with following step

        // Complete sol
        for (CidxAndCost x : red_data.redund_set)
            red_data.cols_to_remove.push_back(x.col);

        // TODO(cava): profile and see if sort + bin-search is faster
        remove_if(sol, [&](cidx_t j) {
            return any(red_data.cols_to_remove, [j](cidx_t r) { return r == j; });
        });

        return true;
    }
};

}  // namespace cft

#endif /* CFT_INCLUDE_GREEDY_HPP */
