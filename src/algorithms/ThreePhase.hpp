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

#ifndef CFT_SRC_ALGORITHMS_THREEPHASE_HPP
#define CFT_SRC_ALGORITHMS_THREEPHASE_HPP


#include "core/Instance.hpp"
#include "core/cft.hpp"
#include "fixing/ColFixing.hpp"
#include "greedy/Greedy.hpp"
#include "subgradient/Subgradient.hpp"
#include "utils/Chrono.hpp"
#include "utils/random.hpp"

namespace cft {

class ThreePhase {
    static constexpr real_t init_step_size = 0.1_F;

    Subgradient subgrad;
    Greedy      greedy;
    ColFixing   col_fixing;

public:
    struct ThreePhaseResult {
        Solution sol;

        // TODO(any): consider putting together multipliers and LB in a struct
        // TODO(any): opposite of the previous todo, the LB can be computed on-the-fly in the
        // refinement procedure starting from the multipliers (but this probably means keeping a
        // useless copy of instance only for this computation, so maybe it's better to store it
        // here)
        std::vector<real_t> unfixed_lagr_mult;
        real_t              unfixed_lb;
    };

    ThreePhaseResult operator()(Environment const& env, Instance& inst) {
        constexpr ridx_t min_row_coverage = 5_R;

        auto tot_timer = Chrono<>();

        auto core = _build_tentative_core_instance(inst, min_row_coverage);

        // First iter data (without fixing) needed by Refinement
        auto   unfixed_lagr_mult = std::vector<real_t>();
        real_t unfixed_lb        = limits<real_t>::min();
        ridx_t orig_nrows        = rsize(inst.rows);  // Save original number of rows

        auto fixing    = FixingData();
        auto pricer    = Pricer();
        auto lagr_mult = std::vector<real_t>(rsize(core.inst.rows), 0.0_F);
        auto sol       = Solution();
        greedy(core.inst, lagr_mult, core.inst.costs, sol);
        auto best_sol = sol;
        _compute_greedy_multipliers(core.inst, lagr_mult);

        CFT_IF_DEBUG(auto inst_copy = inst);
        make_identity_fixing_data(csize(inst.cols), rsize(inst.rows), fixing);

        for (size_t iter_counter = 0; !inst.rows.empty(); ++iter_counter) {
            auto timer = Chrono<>();
            print<3>(env, "3PHS  > Starting 3-phase iteration {}:\n", iter_counter);

            real_t step_size = init_step_size;
            auto   cutoff    = best_sol.cost - fixing.fixed_cost;
            auto   real_lb   = subgrad(env, inst, cutoff, core, step_size, lagr_mult);

            if (iter_counter == 0) {
                unfixed_lagr_mult = lagr_mult;
                unfixed_lb        = real_lb;
            }

            if (real_lb + fixing.fixed_cost >= best_sol.cost - env.epsilon ||
                env.timer.elapsed<sec>() > env.time_limit)
                break;

            sol.idxs.clear();
            sol.cost = cutoff;  // Sol get filled only if one with cost < cutoff is found
            subgrad.heuristic(env, core.inst, step_size, greedy, sol, lagr_mult);

            if (sol.cost + fixing.fixed_cost < best_sol.cost) {
                _from_core_to_unfixed_sol(sol, core, fixing, best_sol);
                CFT_IF_DEBUG(check_solution(inst_copy, best_sol));
            }

            col_fixing(env, orig_nrows, inst, fixing, lagr_mult, greedy);  // Fix column in inst
            real_lb = pricer(inst, lagr_mult, core);        // Update core-inst for next iter
            _perturb_lagr_multipliers(lagr_mult, env.rnd);  // Multipliers +-10% perturbation

            print<3>(env, "3PHS  > Ending iteration    {}:\n", iter_counter);
            print<3>(env, "3PHS  > Remaining rows:     {}\n", rsize(inst.rows));
            print<3>(env, "3PHS  > Remaining columns:  {}\n", csize(inst.cols));
            print<3>(env, "3PHS  > Core instance cols: {}\n", csize(core.inst.cols));
            print<3>(env, "3PHS  > Fixed cost:         {:.2f}\n", fixing.fixed_cost);
            print<3>(env, "3PHS  > Best solution:      {:.2f}\n", best_sol.cost);
            print<3>(env, "3PHS  > Current LB:         {:.2f}\n", real_lb + fixing.fixed_cost);
            print<3>(env, "3PHS  > Iteration time:     {:.2f}s\n", timer.elapsed<sec>());

            // For some reason, it seems that we get the tightest bound after the column fixing
            if (real_lb + fixing.fixed_cost >= best_sol.cost - env.epsilon)
                break;
        }

        print<3>(env,
                 "3PHS  > Best solution: {:.2f}, time: {:.2f}s\n",
                 best_sol.cost,
                 tot_timer.elapsed<sec>());
        return {std::move(best_sol), std::move(unfixed_lagr_mult), unfixed_lb};
    }

private:
    // Transform a solution of a core instance (i.e., where both fixing and pricing have been
    // applied) to the original instance whole without fixing.
    static void _from_core_to_unfixed_sol(Solution const&   core_sol,    // in
                                          InstAndMap const& core,        // in
                                          FixingData const& fixing,      // in
                                          Solution&         unfixed_sol  // out
    ) {
        unfixed_sol.cost = core_sol.cost + fixing.fixed_cost;
        unfixed_sol.idxs = fixing.fixed_cols;
        for (cidx_t j : core_sol.idxs) {
            cidx_t unprice_j = core.col_map[j];
            cidx_t unfixed_j = fixing.curr2orig.col_map[unprice_j];
            unfixed_sol.idxs.push_back(unfixed_j);
        }
    }

    // Greedily creates lagrangian multipliers for the given instance.
    static void _compute_greedy_multipliers(Instance const& inst, std::vector<real_t>& lagr_mult) {

        lagr_mult.assign(rsize(inst.rows), limits<real_t>::max());
        for (ridx_t i = 0_R; i < rsize(inst.rows); ++i)
            for (cidx_t j : inst.rows[i]) {
                real_t candidate = inst.costs[j] / as_real(inst.cols[j].size());
                lagr_mult[i]     = cft::min(lagr_mult[i], candidate);
            }
    }

    // Defines lagrangian multipliers as a perturbation of the given ones.
    static void _perturb_lagr_multipliers(std::vector<real_t>& lagr_mult, prng_t& rnd) {
        for (real_t& u : lagr_mult) {
            u *= rnd_real(rnd, 0.9_F, 1.1_F);
            assert(std::isfinite(u) && "Multiplier is not finite");
        }
    }

    static InstAndMap _build_tentative_core_instance(Instance const& inst,
                                                     size_t          min_row_coverage) {
        ridx_t nrows         = rsize(inst.rows);
        auto   core_inst     = Instance{};
        auto   selected_cols = std::vector<cidx_t>();

        // Select the first n columns of each row (there might be duplicates)
        selected_cols.reserve(nrows * min_row_coverage);
        for (auto const& row : inst.rows)
            for (size_t n = 0; n < min(row.size(), min_row_coverage); ++n) {
                cidx_t j = row[n];  // column covering row i
                selected_cols.push_back(j);
            }

        // There might be duplicates, so let's sort the column list to detect them
        cft::sort(selected_cols);
        cidx_t w     = 0_C;
        cidx_t old_j = removed_cidx;  // To detect duplicates
        for (cidx_t j : selected_cols) {
            if (j == old_j)
                continue;  // Skip duplicate
            old_j              = j;
            selected_cols[w++] = j;                  // Store 1 column per set of duplicates
            push_back_col_from(inst, j, core_inst);  // Add column to core_inst
        }
        selected_cols.resize(w);

        fill_rows_from_cols(core_inst.cols, nrows, core_inst.rows);
        return {std::move(core_inst), std::move(selected_cols)};
    }
};
}  // namespace cft


#endif /* CFT_SRC_ALGORITHMS_THREEPHASE_HPP */
