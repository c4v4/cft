// SPDX-FileCopyrightText: 2024 Francesco Cavaliere <francescocava95@gmail.com>
// SPDX-License-Identifier: MIT

#ifndef CFT_SRC_ALGORITHMS_REFINEMENT_HPP
#define CFT_SRC_ALGORITHMS_REFINEMENT_HPP


#include "algorithms/ThreePhase.hpp"
#include "core/cft.hpp"
#include "utils/Chrono.hpp"
#include "utils/CoverCounters.hpp"
#include "utils/limits.hpp"
#include "utils/utility.hpp"

namespace cft {
namespace local { namespace {

    // Converts the solution of a fixed instance to the solution of the associated unfixed instance.
    inline void from_fixed_to_unfixed_sol(Solution const&   sol,      // in
                                          FixingData const& fixing,   // in
                                          Solution&         best_sol  // out
    ) {
        best_sol.cost = sol.cost + fixing.fixed_cost;
        best_sol.idxs = fixing.fixed_cols;
        for (cidx_t j : sol.idxs)
            best_sol.idxs.push_back(fixing.curr2orig.col_map[j]);
    }

    class RefinementFixManager {

        real_t                   fix_fraction = 0.0_F;
        real_t                   prev_cost    = limits<real_t>::max();
        CoverCounters            row_coverage;
        std::vector<CidxAndCost> gap_contributions;  // Delta values in the paper


    public:
        // Finds a set of columns to fix in the next refinement iteration.
        std::vector<cidx_t> operator()(Environment const&         env,             // in
                                       Instance const&            inst,            // in
                                       std::vector<real_t> const& best_lagr_mult,  // in
                                       Solution const&            best_sol         // in
        ) {
            ridx_t const nrows = rsize(inst.rows);

            fix_fraction = min(1.0_F, fix_fraction * env.alpha);
            if (best_sol.cost < prev_cost)
                fix_fraction = env.min_fixing;
            prev_cost = best_sol.cost;

            auto nrows_real   = as_real(rsize(inst.rows));
            auto nrows_to_fix = checked_cast<ridx_t>(nrows_real * fix_fraction);

            assert(rsize(best_lagr_mult) == rsize(inst.rows));
            assert(nrows_to_fix <= rsize(inst.rows));

            row_coverage.reset(nrows);
            for (cidx_t j : best_sol.idxs)
                row_coverage.cover(inst.cols[j]);

            gap_contributions.clear();
            for (cidx_t j : best_sol.idxs) {
                real_t gap_contrib  = 0.0_F;
                real_t reduced_cost = inst.costs[j];
                for (ridx_t i : inst.cols[j]) {
                    real_t cov = as_real(row_coverage[i]);
                    gap_contrib += best_lagr_mult[i] * (cov - 1.0_F) / cov;
                    reduced_cost -= best_lagr_mult[i];
                }
                gap_contrib += max(reduced_cost, 0.0_F);
                gap_contributions.push_back({j, gap_contrib});
            }
            cft::sort(gap_contributions, [](CidxAndCost c) { return c.cost; });

            ridx_t covered_rows = 0_R;
            row_coverage.reset(nrows);
            auto cols_to_fix = std::vector<cidx_t>();
            for (CidxAndCost c : gap_contributions) {
                covered_rows += as_ridx(row_coverage.cover(inst.cols[c.idx]));
                if (covered_rows > nrows_to_fix)
                    break;
                cols_to_fix.push_back(c.idx);
            }
            return cols_to_fix;
        }
    };
}  // namespace
}  // namespace local

// Complete CFT algorithm (Refinement + call to 3-phase)
inline Solution run(Environment const& env,                // in
                    Instance const&    orig_inst,          // in
                    Solution const&    warmstart_sol = {}  // in
) {

    cidx_t const ncols = csize(orig_inst.cols);
    ridx_t const nrows = rsize(orig_inst.rows);

    auto inst     = orig_inst;
    auto best_sol = Solution();
    best_sol.cost = limits<real_t>::max();

    if (!warmstart_sol.idxs.empty())
        best_sol = warmstart_sol;

    auto three_phase        = ThreePhase();
    auto select_cols_to_fix = local::RefinementFixManager();
    auto nofix_lagr_mult    = std::vector<real_t>();
    auto nofix_lb           = limits<real_t>::max();
    auto old2new            = IdxsMaps();
    auto fixing             = FixingData();
    auto max_cost           = limits<real_t>::max();
    make_identity_fixing_data(ncols, nrows, fixing);
    for (size_t iter_counter = 0;; ++iter_counter) {

        auto result_3p = three_phase(env, inst);
        if (result_3p.sol.cost + fixing.fixed_cost < best_sol.cost) {
            local::from_fixed_to_unfixed_sol(result_3p.sol, fixing, best_sol);
            CFT_IF_DEBUG(check_inst_solution(orig_inst, best_sol));
        }

        if (iter_counter == 0) {
            nofix_lagr_mult = std::move(result_3p.nofix_lagr_mult);
            nofix_lb        = result_3p.nofix_lb;
            max_cost        = env.beta * nofix_lb + env.epsilon;
        }

        if (best_sol.cost <= max_cost || env.timer.elapsed<sec>() > env.time_limit)
            break;

        inst             = orig_inst;
        auto cols_to_fix = select_cols_to_fix(env, inst, nofix_lagr_mult, best_sol);
        if (!cols_to_fix.empty()) {
            make_identity_fixing_data(ncols, nrows, fixing);
            fix_columns_and_compute_maps(cols_to_fix, inst, fixing, old2new);
        }
        real_t nrows_real = as_real(rsize(orig_inst.rows));
        real_t free_perc  = as_real(rsize(inst.rows)) * 100.0_F / nrows_real;
        print<2>(env,
                 "REFN> {:2}: Best solution {:.2f}, lb {:.2f}, gap {:.2f}%\n",
                 iter_counter,
                 best_sol.cost,
                 nofix_lb,
                 100.0_F * (best_sol.cost - nofix_lb) / best_sol.cost);
        print<2>(env,
                 "REFN> {:2}: Fixed cost {:.2f}, free rows {:.0f}%, time {:.2f}s\n\n",
                 iter_counter,
                 fixing.fixed_cost,
                 free_perc,
                 env.timer.elapsed<sec>());

        if (inst.rows.empty() || env.timer.elapsed<sec>() > env.time_limit)
            break;
    }
    best_sol.lower_bound = nofix_lb;
    return best_sol;
}

}  // namespace cft


#endif /* CFT_SRC_ALGORITHMS_REFINEMENT_HPP */
