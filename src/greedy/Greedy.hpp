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

struct Greedy {
    // Caches
    Sorter         sorter;
    Scores         scores;
    RedundancyData red_set;

    real_t operator()(Instance const&            inst,
                      std::vector<real_t> const& lagr_mult,
                      std::vector<cidx_t>&       sol,
                      real_t                     upper_bound  = limits<real_t>::max(),
                      cidx_t                     max_sol_size = limits<cidx_t>::max()) {

        ridx_t to_cover = inst.rows.size();
        red_set.cover_bits.reset(inst.rows.size());
        for (cidx_t j : sol)
            to_cover -= red_set.cover_bits.cover(inst.cols[j]);

        scores.init_scores(inst, lagr_mult, red_set.cover_bits, sol.size(), sorter);
        while (to_cover > 0 && sol.size() < max_sol_size) {
            cidx_t jstar = scores.extract_minscore_col(inst, lagr_mult, red_set.cover_bits, sorter);
            sol.push_back(jstar);
            to_cover -= red_set.cover_bits.cover(inst.cols[jstar]);
        }

        return _remove_redundant_cols(inst, sol, upper_bound);
    }

private:
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
