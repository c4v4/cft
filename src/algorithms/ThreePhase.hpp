// SPDX-FileCopyrightText: 2024 Francesco Cavaliere <francescocava95@gmail.com>
// SPDX-License-Identifier: MIT

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

struct ThreePhaseResult {
    Solution            sol;              // Best feasible solution found
    std::vector<real_t> nofix_lagr_mult;  // Lagrangian multipliers before the first fixing
    real_t              nofix_lb;         // Lower bound before the first fixing
};

class ThreePhase {
    static constexpr real_t init_step_size = 0.1_F;

    // Caches
    Subgradient         subgrad;            // Subgradient functor
    Greedy              greedy;             // Greedy functor
    ColFixing           col_fixing;         // Column fixing functor
    Pricer              pricer;             // Pricing functor
    FixingData          fixing;             // Column fixing data
    Solution            sol;                // Current solution
    Solution            best_sol;           // Best solution
    InstAndMap          core;               // Core instance
    std::vector<real_t> lagr_mult;          // Lagrangian multipliers
    std::vector<real_t> unfixed_lagr_mult;  // Best multipliers before the first fixing

public:
    // 3-phase algorithm consisting in subgradient, greedy and column fixing.
    // NOTE: inst gets progressively fixed inplace, loosing its original state.
    ThreePhaseResult operator()(Environment const& env,  // in
                                Instance&          inst  // in/cache
    ) {
        ridx_t const orig_nrows = rsize(inst.rows);  // Original number of rows for ColFixing

        auto tot_timer  = Chrono<>();
        auto unfixed_lb = limits<real_t>::min();
        _three_phase_setup(inst, greedy, sol, best_sol, core, lagr_mult, fixing);

        CFT_IF_DEBUG(auto inst_copy = inst);
        for (size_t iter_counter = 0; !inst.rows.empty(); ++iter_counter) {
            auto timer = Chrono<>();
            print<3>(env, "3PHS> Three-phase iteration {}:\n", iter_counter);

            real_t step_size = init_step_size;
            auto   cutoff    = best_sol.cost - fixing.fixed_cost;
            auto   real_lb   = subgrad(env, inst, cutoff, pricer, core, step_size, lagr_mult);

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
                CFT_IF_DEBUG(check_inst_solution(inst_copy, best_sol));
            }

            col_fixing(env, orig_nrows, inst, fixing, lagr_mult, greedy);  // Fix column in inst
            real_lb = pricer(inst, lagr_mult, core);        // Update core-inst for next iter
            _perturb_lagr_multipliers(lagr_mult, env.rnd);  // Multipliers +-10% perturbation

            print<3>(env, "3PHS> Remaining rows:     {}\n", rsize(inst.rows));
            print<3>(env, "3PHS> Remaining columns:  {}\n", csize(inst.cols));
            print<3>(env, "3PHS> Core instance cols: {}\n", csize(core.inst.cols));
            print<3>(env, "3PHS> Fixed cost:         {:.2f}\n", fixing.fixed_cost);
            print<3>(env, "3PHS> Best solution:      {:.2f}\n", best_sol.cost);
            print<3>(env, "3PHS> Current LB:         {:.2f}\n", real_lb + fixing.fixed_cost);
            print<3>(env, "3PHS> Iteration time:     {:.2f}s\n\n", timer.elapsed<sec>());

            // For some reason, it seems that we get the tightest bound after the column fixing
            if (real_lb + fixing.fixed_cost >= best_sol.cost - env.epsilon)
                break;
        }

        print<3>(env,
                 "3PHS> Best solution: {:.2f}, time: {:.2f}s\n\n",
                 best_sol.cost,
                 tot_timer.elapsed<sec>());
        return {best_sol, unfixed_lagr_mult, unfixed_lb};
    }

private:
    static void _three_phase_setup(Instance const&      inst,       // in
                                   Greedy&              greedy,     // cache
                                   Solution&            sol,        // cache
                                   Solution&            best_sol,   // out
                                   InstAndMap&          core,       // out
                                   std::vector<real_t>& lagr_mult,  // out
                                   FixingData&          fixing      // out
    ) {
        _build_tentative_core_instance(inst, core);         // init core instance
        _compute_greedy_multipliers(core.inst, lagr_mult);  // compute initial multipliers
        make_identity_fixing_data(csize(inst.cols), rsize(inst.rows), fixing);  // init fixing

        // init sol
        sol.idxs.clear();
        sol.cost = greedy(core.inst, lagr_mult, core.inst.costs, sol.idxs);

        _from_core_to_unfixed_sol(sol, core, fixing, best_sol);  // init best_sol
        CFT_IF_DEBUG(check_inst_solution(inst, best_sol));
    }

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
    static void _compute_greedy_multipliers(Instance const&      inst,      // in
                                            std::vector<real_t>& lagr_mult  // out
    ) {

        lagr_mult.assign(rsize(inst.rows), limits<real_t>::max());
        for (ridx_t i = 0_R; i < rsize(inst.rows); ++i)
            for (cidx_t j : inst.rows[i]) {
                real_t candidate = inst.costs[j] / as_real(inst.cols[j].size());
                lagr_mult[i]     = cft::min(lagr_mult[i], candidate);
            }
    }

    // Defines lagrangian multipliers as a perturbation of the given ones.
    static void _perturb_lagr_multipliers(std::vector<real_t>& lagr_mult,  // inout
                                          prng_t&              rnd         // inout
    ) {
        for (real_t& u : lagr_mult) {
            u *= rnd_real(rnd, 0.9_F, 1.1_F);
            assert(std::isfinite(native_cast(u)) && "Multiplier is not finite");
        }
    }

    static void _build_tentative_core_instance(Instance const& inst,      // in
                                               InstAndMap&     core_inst  // out
    ) {
        static constexpr cidx_t min_row_coverage = 5_C;
        ridx_t const            nrows            = rsize(inst.rows);

        clear_inst(core_inst.inst);
        core_inst.col_map.clear();

        // Select the first n columns of each row (there might be duplicates)
        core_inst.col_map.reserve(checked_cast<size_t>(as_cidx(nrows) * min_row_coverage));
        for (auto const& row : inst.rows)
            for (size_t n = 0; n < min(row.size(), min_row_coverage); ++n) {
                cidx_t j = row[n];  // column covering row i
                core_inst.col_map.push_back(j);
            }

        // There might be duplicates, so let's sort the column list to detect them
        cft::sort(core_inst.col_map);
        cidx_t w     = 0_C;
        cidx_t old_j = removed_cidx;  // To detect duplicates
        for (cidx_t j : core_inst.col_map) {
            if (j == old_j)
                continue;  // Skip duplicate
            old_j                  = j;
            core_inst.col_map[w++] = j;                   // Store 1 column per set of duplicates
            push_back_col_from(inst, j, core_inst.inst);  // Add column to core_inst
        }
        core_inst.col_map.resize(w);

        fill_rows_from_cols(core_inst.inst.cols, nrows, core_inst.inst.rows);
    }
};
}  // namespace cft


#endif /* CFT_SRC_ALGORITHMS_THREEPHASE_HPP */
