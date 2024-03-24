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

#ifndef CAV_SRC_ALGORITHMS_THREEPHASE_HPP
#define CAV_SRC_ALGORITHMS_THREEPHASE_HPP

#include "core/cft.hpp"
#include "core/random.hpp"
#include "fixing/ColFixing.hpp"
#include "fixing/fix_columns.hpp"
#include "greedy/Greedy.hpp"
#include "instance/Instance.hpp"
#include "subgradient/subgradient.hpp"

namespace cft {
struct ThreePhase {
private:
    Greedy    greedy;
    ColFixing col_fixing;
    Sorter    sorter;

    static void _convert_to_orig_sol(Solution const&   sol,
                                     FixingData const& fixing,
                                     Solution&         best_sol) {
        best_sol.cost = sol.cost + fixing.fixed_cost;
        best_sol.idxs = fixing.fixed_cols;
        for (cidx_t j : sol.idxs)
            best_sol.idxs.push_back(fixing.curr2orig_col_map[j]);
    }

public:
    Solution operator()(Instance& inst, prng_t& rnd) {

        auto best_sol = Solution();
        auto fixing   = make_identity_fixing_data(inst.cols.size(), inst.rows.size());

        while (!inst.rows.empty()) {
            constexpr ridx_t min_row_coverage = 5;

            auto core      = build_tentative_core_instance(inst, sorter, min_row_coverage);
            auto lagr_mult = compute_greedy_multipliers(core.inst);
            auto sol       = Solution();
            greedy(core.inst, lagr_mult, sol);

            real_t step_size = 0.1;
            auto   cutoff    = std::min(sol.cost, best_sol.cost - fixing.fixed_cost);

            auto sg_lb = subgradient(inst, core, sorter, cutoff, sol.cost, step_size, lagr_mult);
            if (sg_lb >= cutoff - CFT_EPSILON)
                break;

            auto heur_lb = heuristic(core.inst, greedy, cutoff, step_size, sol, lagr_mult);
            if (heur_lb >= cutoff - CFT_EPSILON)
                break;

            fmt::print("3PHS > Lower bound: {}, Solution cost: {}\n",
                       max(sg_lb, heur_lb) + fixing.fixed_cost,
                       sol.cost + fixing.fixed_cost);

            if (sol.cost + fixing.fixed_cost < best_sol.cost)
                _convert_to_orig_sol(sol, fixing, best_sol);

            col_fixing(inst, core, fixing, lagr_mult, greedy);
            fmt::print("3PHS > Fixing: rows left: {}, fixed cost: {}\n",
                       inst.rows.size(),
                       fixing.fixed_cost);
            perturb_lagr_multipliers(lagr_mult, rnd);
        }

        fmt::print("\n3PHS > Best solution cost: {}\n", best_sol.cost);
        return best_sol;
    }
};
}  // namespace cft
#endif /* CAV_SRC_ALGORITHMS_THREEPHASE_HPP */
