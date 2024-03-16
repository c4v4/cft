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

#ifndef CFT_INCLUDE_GREEDY_HPP
#define CFT_INCLUDE_GREEDY_HPP

#include "Scores.hpp"
#include "core/cft.hpp"
#include "core/coverage.hpp"
#include "core/limits.hpp"
#include "core/sort.hpp"
#include "core/utility.hpp"
#include "instance/Instance.hpp"
#include "redundancy.hpp"

namespace cft {

/// @brief This is the greedy step of the 3-phase of the CFT algorithm. It uses a set of Lagrangian
/// multipliers to attempt to find an decent solution for the SCP problem (quantity over quality).
/// For efficiency, we use a function object instead of a simple function. This allows us to cache
/// data structures used in the Greedy step, preventing repeated reallocations. It also keeps
/// internal details hidden, unlike using an external struct for cache storage.
/// At the end of the day though, it can be mentally thought of as a function.
struct Greedy {
    // Caches
    Sorter         sorter;
    Scores         scores;
    RedundancyData red_set;

    /// @brief The greedy algorithm:
    /// 1. Initialize column scores (based on the current lagragian multipliers)
    /// 2. Add the column with the best score (until the solution is "complete")
    /// 3. If present, remove redundant columns from the solution
    real_t operator()(Instance const&            inst,
                      std::vector<real_t> const& lagr_mult,
                      std::vector<cidx_t>&       sol,
                      real_t                     upper_bound  = limits<real_t>::max(),
                      cidx_t                     max_sol_size = limits<cidx_t>::max()) {

        ridx_t nrows_to_cover = inst.rows.size();
        red_set.curr_cover.reset(inst.rows.size());

        if (sol.empty())
            scores.init_scores(inst, lagr_mult, sorter);
        else {
            for (cidx_t j : sol)
                nrows_to_cover -= red_set.curr_cover.cover(inst.cols[j]);
            scores.init_scores(inst, lagr_mult, red_set.curr_cover, sorter);
        }

        while (nrows_to_cover > 0 && sol.size() < max_sol_size) {
            cidx_t jstar = scores.extract_minscore_col(inst, lagr_mult, red_set.curr_cover, sorter);
            sol.push_back(jstar);
            nrows_to_cover -= red_set.curr_cover.cover(inst.cols[jstar]);
        }

        return _remove_redundant_cols(inst, sol, upper_bound);
    }

private:
    /// @brief Remove redundant columns from the solution. Over a certain threshold, columns are
    /// removed heuristically, under that threshold, they are removed by enumeration.
    real_t _remove_redundant_cols(Instance const& inst, std::vector<cidx_t>& sol, real_t ub) {

        real_t curr_sol_cost = init_redund_set(red_set, inst, sol, ub, sorter);
        if (curr_sol_cost >= ub || red_set.redund_set.empty())
            return curr_sol_cost;

        curr_sol_cost = heuristic_removal(red_set, inst, curr_sol_cost);
        if (curr_sol_cost >= ub || red_set.redund_set.empty())
            return curr_sol_cost;

        curr_sol_cost = enumeration_removal(red_set, inst, curr_sol_cost);
        if (curr_sol_cost >= ub)
            return curr_sol_cost;

        remove_if(sol, [&](cidx_t j) {
            return any(red_set.cols_to_remove, [j](cidx_t r) { return r == j; });
        });

        return curr_sol_cost;
    }
};

inline Greedy make_greedy() {
    return Greedy{make_sorter(), make_greedy_scores(), make_redundancy_data()};
}

}  // namespace cft

#endif /* CFT_INCLUDE_GREEDY_HPP */
