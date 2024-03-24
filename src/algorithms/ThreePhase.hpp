// Copyright (c) 2024 Luca Accorsi and Francesco Cavaliere
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

#ifndef CFT_SRC_ALGORITHMS_THREEPHASE_HPP
#define CFT_SRC_ALGORITHMS_THREEPHASE_HPP

#include "core/Chrono.hpp"
#include "core/cft.hpp"
#include "core/random.hpp"
#include "fixing/ColFixing.hpp"
#include "fixing/fix_columns.hpp"
#include "greedy/Greedy.hpp"
#include "instance/Instance.hpp"
#include "subgradient/subgradient.hpp"

namespace cft {
namespace {
    inline void convert_to_orig_sol(Solution const&   sol,
                                    FixingData const& fixing,
                                    Solution&         best_sol) {
        best_sol.cost = sol.cost + fixing.fixed_cost;
        best_sol.idxs = fixing.fixed_cols;
        for (cidx_t j : sol.idxs)
            best_sol.idxs.push_back(fixing.curr2orig.col_map[j]);
    }

    // Greedily creates lagrangian multipliers for the given instance.
    inline std::vector<real_t> compute_greedy_multipliers(Instance const& inst) {
        auto lagr_mult = std::vector<real_t>(inst.rows.size(), limits<real_t>::max());

        for (size_t i = 0; i < inst.rows.size(); ++i)
            for (cidx_t const j : inst.rows[i]) {
                real_t const candidate = inst.costs[j] / static_cast<real_t>(inst.cols[j].size());
                lagr_mult[i]           = cft::min(lagr_mult[i], candidate);
            }

        return lagr_mult;
    }

    inline InstAndMap build_tentative_core_instance(Instance const& inst,
                                                    Sorter&         sorter,
                                                    size_t          min_row_coverage) {
        ridx_t nrows        = inst.rows.size();
        auto   core_inst    = Instance{};
        auto   core_col_map = std::vector<cidx_t>();

        core_col_map.reserve(nrows * min_row_coverage);
        for (auto const& row : inst.rows)
            for (size_t n = 0; n < min(row.size(), min_row_coverage); ++n)
                core_col_map.push_back(row[n]);

        sorter.sort(core_col_map);
        cidx_t w      = 0;
        cidx_t prev_j = CFT_REMOVED_IDX;
        for (cidx_t j : core_col_map) {
            if (j == prev_j)
                continue;
            prev_j            = j;
            core_col_map[w++] = j;
            core_inst.cols.push_back(inst.cols[j]);
            core_inst.costs.push_back(inst.costs[j]);
            core_inst.solcosts.push_back(inst.solcosts[j]);
        }
        core_col_map.resize(w);

        fill_rows_from_cols(core_inst.cols, nrows, core_inst.rows);
        return {std::move(core_inst), std::move(core_col_map)};
    }

}  // namespace

struct ThreePhase {
private:
    Subgradient subgrad;
    Greedy      greedy;
    ColFixing   col_fixing;
    Sorter      sorter;

public:
    Solution operator()(Instance& inst, prng_t& rnd) {
        constexpr ridx_t min_row_coverage = 5;

        auto tot_timer = Chrono<>();
        auto best_sol  = Solution();
        auto fixing    = make_identity_fixing_data(inst.cols.size(), inst.rows.size());

        size_t iter_counter = 0;
        while (!inst.rows.empty()) {
            fmt::print("3PHS > Starting 3-phase iteration: {}\n", ++iter_counter);

            auto core      = build_tentative_core_instance(inst, sorter, min_row_coverage);
            auto lagr_mult = compute_greedy_multipliers(core.inst);
            auto sol       = Solution();
            greedy(core.inst, lagr_mult, sol);

            real_t step_size = 0.1;
            auto   cutoff    = std::min(sol.cost, best_sol.cost - fixing.fixed_cost);

            auto real_lb = subgrad(inst, core, sorter, cutoff, sol.cost, step_size, lagr_mult);
            if (real_lb >= cutoff - CFT_EPSILON)
                break;

            subgrad.heuristic(core.inst, greedy, cutoff, step_size, sol, lagr_mult);

            fmt::print("3PHS > Lower bound: {}, Solution cost: {}\n",
                       real_lb + fixing.fixed_cost,
                       sol.cost + fixing.fixed_cost);

            if (sol.cost + fixing.fixed_cost < best_sol.cost)
                convert_to_orig_sol(sol, fixing, best_sol);

            col_fixing(inst, core, fixing, lagr_mult, greedy);
            fmt::print("3PHS > Fixing: rows left: {}, fixed cost: {:.2f}\n",
                       inst.rows.size(),
                       fixing.fixed_cost);
            perturb_lagr_multipliers(lagr_mult, rnd);
            fmt::print("3PHS > Finished iteration {}, time {:.2f}s\n",
                       iter_counter,
                       timer.elapsed<sec>());
        }

        fmt::print("\n3PHS > Best solution cost: {:.2f}, time: {:.2f}s\n",
                   best_sol.cost,
                   tot_timer.elapsed<sec>());
        return best_sol;
    }
};
}  // namespace cft
#endif /* CFT_SRC_ALGORITHMS_THREEPHASE_HPP */
