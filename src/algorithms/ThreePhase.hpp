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


#include "core/Chrono.hpp"
#include "core/cft.hpp"
#include "core/random.hpp"
#include "fixing/ColFixing.hpp"
#include "greedy/Greedy.hpp"
#include "instance/Instance.hpp"
#include "subgradient/subgradient.hpp"
#include "subgradient/utils.hpp"

namespace cft {

// Simply transform a solution of an instance with fixing, to a solution of the original
// instance without fixing
inline void from_fixed_to_unfixed_sol(Solution const&   fixed_sol,   // in
                                      FixingData const& fixing,      // in
                                      Solution&         unfixed_sol  // out
) {
    unfixed_sol.cost = fixed_sol.cost + fixing.fixed_cost;
    unfixed_sol.idxs = fixing.fixed_cols;
    for (cidx_t j : fixed_sol.idxs)
        unfixed_sol.idxs.push_back(fixing.curr2orig.col_map[j]);
}

namespace local { namespace {
    // Transform a solution of a core instance (i.e., where both fixing and pricing have been
    // applied) to the original instance whole without fixing.
    inline void from_core_to_unfixed_sol(Solution const&   core_sol,    // in
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
    inline std::vector<real_t> compute_greedy_multipliers(Instance const& inst) {
        auto lagr_mult = std::vector<real_t>(inst.rows.size(), limits<real_t>::max());

        for (size_t i = 0; i < inst.rows.size(); ++i)
            for (cidx_t j : inst.rows[i]) {
                real_t candidate = inst.costs[j] / static_cast<real_t>(inst.cols[j].size());
                lagr_mult[i]     = cft::min(lagr_mult[i], candidate);
            }

        return lagr_mult;
    }

    inline InstAndMap build_tentative_core_instance(Instance const& inst,
                                                    Sorter&         sorter,
                                                    size_t          min_row_coverage) {
        ridx_t nrows         = inst.rows.size();
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
        sorter.sort(selected_cols);
        cidx_t w     = 0;
        cidx_t old_j = CFT_REMOVED_IDX;  // To detect duplicates
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

}  // namespace
}  // namespace local

struct ThreePhaseResult {
    Solution sol;

    // TODO(any): consider putting together multipliers and LB in a struct
    // TODO(any): opposite of the previous todo, the LB can be computed on-the-fly in the refinement
    // procedure starting from the multipliers (but this probably means keeping a useless copy of
    // instance only for this computation, so maybe it's better to store it here)
    std::vector<real_t> unfixed_lagr_mult;
    real_t              unfixed_lb;
};

struct ThreePhase {
private:
    Subgradient subgrad;
    Greedy      greedy;
    ColFixing   col_fixing;
    Sorter      sorter;

public:
    ThreePhaseResult operator()(Instance& inst, prng_t& rnd, double tlim) {
        constexpr ridx_t min_row_coverage = 5;

        auto tot_timer = Chrono<>();

        auto core      = local::build_tentative_core_instance(inst, sorter, min_row_coverage);
        auto lagr_mult = local::compute_greedy_multipliers(core.inst);

        // First iter data (without dixing) needed by Refinement
        auto   unfixed_lagr_mult = std::vector<real_t>();
        real_t unfixed_lb        = limits<real_t>::min();

        auto fixing = FixingData();
        auto pricer = Pricer();
        auto sol    = Solution();
        greedy(core.inst, lagr_mult, sol);
        auto best_sol = sol;

        IF_DEBUG(auto inst_copy = inst);
        make_identity_fixing_data(inst.cols.size(), inst.rows.size(), fixing);

        for (size_t iter_counter = 0; !inst.rows.empty(); ++iter_counter) {
            auto timer = Chrono<>();
            fmt::print("3PHS > Starting 3-phase iteration {}:\n", iter_counter);

            real_t step_size = 0.1;
            auto   cutoff    = best_sol.cost - fixing.fixed_cost;
            auto   real_lb   = subgrad(inst, cutoff, sorter, core, step_size, lagr_mult);

            if (iter_counter == 0) {
                unfixed_lagr_mult = lagr_mult;
                unfixed_lb        = real_lb;
            }

            if (real_lb + fixing.fixed_cost >= best_sol.cost - CFT_EPSILON ||
                tot_timer.elapsed<sec>() > tlim)
                break;

            sol.idxs.clear();
            sol.cost = cutoff;  // sol get filled only if one with cost < cutoff is found
            subgrad.heuristic(core.inst, step_size, greedy, sol, lagr_mult);

            if (sol.cost + fixing.fixed_cost < best_sol.cost) {
                local::from_core_to_unfixed_sol(sol, core, fixing, best_sol);
                IF_DEBUG(check_solution(inst_copy, best_sol));
            }

            col_fixing(inst, fixing, lagr_mult, greedy);  // Fix column in inst
            real_lb = pricer(inst, lagr_mult, core);      // Update core-inst for next iter
            perturb_lagr_multipliers(lagr_mult, rnd);     // Perturb multipliers to explore more

            fmt::print("3PHS > Ending iteration    {}:\n", iter_counter);
            fmt::print("3PHS > Remaining rows:     {}\n", inst.rows.size());
            fmt::print("3PHS > Remaining columns:  {}\n", inst.cols.size());
            fmt::print("3PHS > Core instance cols: {}\n", core.inst.cols.size());
            fmt::print("3PHS > Fixed cost:         {:.2f}\n", fixing.fixed_cost);
            fmt::print("3PHS > Best solution:      {:.2f}\n", best_sol.cost);
            fmt::print("3PHS > Current LB:         {:.2f}\n", real_lb + fixing.fixed_cost);
            fmt::print("3PHS > Iteration time:     {:.2f}s\n", timer.elapsed<sec>());

            if (real_lb + fixing.fixed_cost >= best_sol.cost - CFT_EPSILON)
                break;
        }

        fmt::print("3PHS > Best solution cost: {:.2f}, time: {:.2f}s\n",
                   best_sol.cost,
                   tot_timer.elapsed<sec>());
        return {std::move(best_sol), std::move(unfixed_lagr_mult), unfixed_lb};
    }
};
}  // namespace cft


#endif /* CFT_SRC_ALGORITHMS_THREEPHASE_HPP */
