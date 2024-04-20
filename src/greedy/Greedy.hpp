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

#ifndef CFT_SRC_GREEDY_GREEDY_HPP
#define CFT_SRC_GREEDY_GREEDY_HPP


#include "core/Instance.hpp"
#include "core/cft.hpp"
#include "greedy/redundancy.hpp"
#include "greedy/scores.hpp"
#include "utils/CoverCounters.hpp"
#include "utils/limits.hpp"
#include "utils/utility.hpp"

namespace cft {

// This is the greedy step of the 3-phase of the CFT algorithm. It uses a set of Lagrangian
// multipliers to attempt to find an decent solution for the SCP problem (quantity over quality).
// For efficiency, we use a function object instead of a simple function. This allows us to cache
// data structures used in the Greedy step, preventing repeated reallocations. It also keeps
// internal details hidden, unlike using an external struct for cache storage. At the end of the day
// though, it can be mentally thought of as a function.
class Greedy {
    // Caches
    Scores         score_info;
    RedundancyData redund_info;

public:
    // The greedy algorithm:
    // 1. Initialize column scores (based on the current lagragian multipliers)
    // 2. Add the column with the best score (until the solution is "complete")
    // 3. If present, remove redundant columns from the solution
    // NOTE: a valid solution is returned only if its cost is below the cutoff_cost
    real_t operator()(Instance const&            inst,                                  // in
                      std::vector<real_t> const& lagr_mult,                             // in
                      std::vector<real_t> const& reduced_costs,                         // in
                      std::vector<cidx_t>&       sol,                                   // out
                      real_t                     cutoff_cost  = limits<real_t>::max(),  // in
                      cidx_t                     max_sol_size = limits<cidx_t>::max()   // in
    ) {
        ridx_t const nrows = rsize(inst.rows);

        real_t sol_cost = limits<real_t>::max();
        if (csize(sol) >= max_sol_size)
            return sol_cost;

        score_info.gammas = reduced_costs;

        auto& total_cover = redund_info.total_cover;
        total_cover.reset(nrows);

        ridx_t nrows_to_cover = nrows;
        complete_scores_init(inst, score_info);
        if (!sol.empty())
            nrows_to_cover -= update_covered(inst, sol, lagr_mult, score_info, total_cover);

        if (nrows_to_cover == 0_R)
            return sol_cost;


        // Fill solution
        auto   good_scores      = score_subspan_t{};
        real_t worst_good_score = 0.0_F;
        while (nrows_to_cover > 0_R && csize(sol) < max_sol_size) {

            // Get the column-fraction with best scores
            if (good_scores.empty()) {
                auto good_size   = min(as_cidx(nrows_to_cover), csize(inst.cols) - csize(sol));
                good_scores      = select_good_scores(score_info, good_size);
                worst_good_score = good_scores.back().score;
            }

            auto   min_score = range_min(good_scores, ScoreKey{});
            cidx_t jstar     = min_score.idx;
            assert(min_score.score < limits<real_t>::max() && "Illegal score");
            assert(!any(sol, [=](cidx_t j) { return j == jstar; }) && "Duplicate column");
            sol.push_back(jstar);

            update_changed_scores(inst, lagr_mult, total_cover, jstar, score_info, [&](cidx_t s) {
                // s score changed, check if can be removed from good_scores
                if (s < csize(good_scores) && good_scores[s].score >= worst_good_score) {
                    cidx_t s_j    = good_scores[s].idx;
                    cidx_t back_j = good_scores.back().idx;
                    std::swap(good_scores[s], good_scores.back());
                    std::swap(score_info.score_map[s_j], score_info.score_map[back_j]);
                    good_scores = make_span(good_scores.begin(), good_scores.end() - 1U);
                }
            });

            nrows_to_cover -= as_ridx(total_cover.cover(inst.cols[jstar]));
        }

        return _remove_redundant_cols(inst, cutoff_cost, redund_info, sol);
    }

private:
    static real_t _remove_redundant_cols(Instance const&      inst,         // in
                                         real_t               cutoff_cost,  // in
                                         RedundancyData&      redund_info,  // inout
                                         std::vector<cidx_t>& sol           // inout
    ) {
        complete_init_redund_set(inst, sol, cutoff_cost, redund_info);
        if (_try_early_exit(redund_info, sol))
            return redund_info.partial_cost;  // Solution will be discarded
        CFT_IF_DEBUG(check_redundancy_data(inst, sol, redund_info));

        heuristic_removal(inst, redund_info);
        if (_try_early_exit(redund_info, sol))
            return redund_info.partial_cost;  // Solution will be discarded
        CFT_IF_DEBUG(check_redundancy_data(inst, sol, redund_info));

        enumeration_removal(inst, redund_info);
        if (redund_info.best_cost < cutoff_cost)
            remove_if(sol, [&](cidx_t j) {
                return any(redund_info.cols_to_remove, [j](cidx_t r) { return r == j; });
            });

        return redund_info.best_cost;
    }

    static bool _try_early_exit(RedundancyData& redund_info, std::vector<cidx_t>& sol) {
        if (redund_info.partial_cost >= redund_info.best_cost || redund_info.redund_set.empty())
            return true;  // Partial solution already worse than best cost

        if (redund_info.partial_cov_count < rsize(redund_info.partial_cover))
            return false;  // Partial cover not completed yet, continue

        // Otherwise complete the solution removing redundant columns
        for (CidxAndCost x : redund_info.redund_set)
            redund_info.cols_to_remove.push_back(x.idx);

        remove_if(sol, [&](cidx_t j) {
            return any(redund_info.cols_to_remove, [j](cidx_t r) { return r == j; });
        });

        return true;
    }
};

}  // namespace cft


#endif /* CFT_SRC_GREEDY_GREEDY_HPP */
