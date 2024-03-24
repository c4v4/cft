// Copyright (c) 2024 Luca Accorsi and Francesco Cavaliere
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
private:
    // Caches
    Sorter         sorter;
    Scores         score_info;
    RedundancyData redund_info;

public:
    void operator()(Instance const&            inst,
                    std::vector<real_t> const& lagr_mult,
                    Solution&                  sol,
                    real_t                     cutoff_cost  = limits<real_t>::max(),
                    cidx_t                     max_sol_size = limits<cidx_t>::max()) {

        score_info.gammas = inst.costs;
        for (cidx_t j = 0; j < inst.cols.size(); ++j)
            for (ridx_t i : inst.cols[j])
                score_info.gammas[j] -= lagr_mult[i];

        operator()(inst, lagr_mult, score_info.gammas, sol, cutoff_cost, max_sol_size);
    }

    // The greedy algorithm:
    // 1. Initialize column scores (based on the current lagragian multipliers)
    // 2. Add the column with the best score (until the solution is "complete")
    // 3. If present, remove redundant columns from the solution
    // NOTE: a valid solution is returned only if its cost is below the cutoff_cost
    void operator()(Instance const&            inst,
                    std::vector<real_t> const& lagr_mult,
                    std::vector<real_t> const& reduced_costs,
                    Solution&                  sol,
                    real_t                     cutoff_cost  = limits<real_t>::max(),
                    cidx_t                     max_sol_size = limits<cidx_t>::max()) {

        score_info.gammas = reduced_costs;

        ridx_t nrows       = inst.rows.size();
        auto&  total_cover = redund_info.total_cover;
        total_cover.reset(nrows);

        ridx_t nrows_to_cover = nrows;
        complete_scores_init(inst, score_info);
        if (!sol.idxs.empty())
            nrows_to_cover -= update_already_covered(inst, sol, lagr_mult, score_info, total_cover);

        auto   smaller_size         = min(nrows_to_cover, inst.cols.size());
        auto   good_scores          = compute_good_scores(sorter, score_info, smaller_size);
        real_t score_update_trigger = good_scores.back().score;

        // Fill solution
        while (nrows_to_cover > 0 && sol.idxs.size() < max_sol_size) {

            cidx_t s_min     = argmin(good_scores, ScoreKey{});
            real_t min_score = good_scores[s_min].score;
            if (min_score >= score_update_trigger) {
                smaller_size         = min(nrows_to_cover, inst.cols.size() - sol.idxs.size());
                good_scores          = compute_good_scores(sorter, score_info, smaller_size);
                score_update_trigger = good_scores.back().score;
                s_min                = argmin(good_scores, ScoreKey{});
            }

            cidx_t jstar = score_info.scores[s_min].idx;
            assert(score_info.scores[s_min].score < limits<real_t>::max() && "Illegal score");
            assert(!any(sol.idxs, [=](cidx_t j) { return j == jstar; }) && "Duplicate column");
            sol.idxs.push_back(jstar);
            sol.cost += inst.costs[jstar];

            update_changed_scores(inst, lagr_mult, total_cover, s_min, score_info);
            nrows_to_cover -= total_cover.cover(inst.cols[jstar]);
        }

        _remove_redundant_cols(inst, redund_info, sorter, cutoff_cost, sol);
    }

private:
    static void _remove_redundant_cols(Instance const& inst,
                                       RedundancyData& redund_info,  // input in .total_cover
                                       Sorter&         sorter,       // cache
                                       real_t          cutoff_cost,  // early exit cutoff
                                       Solution&       sol           // input/output solution
    ) {

        complete_init_redund_set(redund_info, inst, sol.idxs, sorter, cutoff_cost);
        if (_try_early_exit(redund_info, sol))
            return;
        IF_DEBUG(check_redundancy_data(inst, sol.idxs, redund_info));

        heuristic_removal(redund_info, inst);
        if (_try_early_exit(redund_info, sol))
            return;
        IF_DEBUG(check_redundancy_data(inst, sol.idxs, redund_info));

        enumeration_removal(redund_info, inst);
        sol.cost = redund_info.best_cost;
        if (sol.cost >= cutoff_cost)
            return;

        remove_if(sol.idxs, [&](cidx_t j) {
            return any(redund_info.cols_to_remove, [j](cidx_t r) { return r == j; });
        });
    }

    static bool _try_early_exit(RedundancyData& redund_info, Solution& sol) {
        sol.cost = redund_info.partial_cost;
        if (redund_info.partial_cost >= redund_info.best_cost || redund_info.redund_set.empty())
            return true;  // Discard sol

        if (redund_info.partial_cov_count < redund_info.partial_cover.size())
            return false;  // Continue with following step

        // Complete sol
        for (CidxAndCost x : redund_info.redund_set)
            redund_info.cols_to_remove.push_back(x.col);

        // TODO(cava): profile and see if sort + bin-search is faster
        remove_if(sol.idxs, [&](cidx_t j) {
            return any(redund_info.cols_to_remove, [j](cidx_t r) { return r == j; });
        });

        return true;
    }
};

}  // namespace cft

#endif /* CFT_INCLUDE_GREEDY_HPP */
