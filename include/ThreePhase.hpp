#ifndef SCP_INCLUDE_THREEPHASE_HPP_
#define SCP_INCLUDE_THREEPHASE_HPP_

#include <fmt/core.h>

#include <algorithm>
#include <cassert>
#include <vector>

#include "ColumnFixing.hpp"
#include "Greedy.hpp"
#include "Instance.hpp"
#include "Multipliers.hpp"
#include "Solution.hpp"
#include "SubGradient.hpp"
#include "cft.hpp"

#define EXPLORING_ITERS 250

class ThreePhase {
public:
    ThreePhase(SubInstance& subinst_, MStar& covered_rows_, std::mt19937& rnd_)
        : subinst(subinst_),
          covered_rows(covered_rows_),
          subgradient(subinst),
          greedy(subinst, covered_rows_),
          col_fixing(subinst, greedy, covered_rows_),
          rnd(rnd_) { }

    std::pair<GlobalSolution, GlobalMultipliers> operator()(const real_t global_UB) {

        auto glo_u = GlobalMultipliers();
        auto u_star = LocalMultipliers(subinst.get_nrows(), 0.0);
        auto S_star = GlobalSolution(subinst, greedy(u_star));
        auto glo_UB_star = std::min<real_t>(global_UB, S_star.get_cost());

        idx_t remaining_rows = subinst.get_nrows();

        LocalMultipliers u_k = SubGradient::u_greedy_init(subinst);
        idx_t iter = 1;

        do {

            IF_VERBOSE { fmt::print("┌─ 3-PHASE: iter {:2} ────────────────────────────────────────────────────────────────\n", iter); }

            // LocalSolution S_curr = greedy(u_k);
            // real_t S_curr_cost = S_curr.compute_cost(subinst);
            // assert(S_curr_cost > 0);

            const auto subgrad_UB = glo_UB_star - subinst.get_fixed_cost();

            // 1. SUBGRADIENT PHASE
            u_k = subgradient.solve(subgrad_UB, u_k);     // use S_curr_cost because u_k refers to the current sub-problem
            real_t local_LB = subgradient.get_best_LB();  // u_k.compute_lb(subinst);
            real_t global_LB = subinst.get_fixed_cost() + local_LB;
            if (iter == 1) { glo_u = GlobalMultipliers(subinst, u_k); }


            if (global_LB >= glo_UB_star - HAS_INTEGRAL_COSTS) {
                IF_VERBOSE {
                    fmt::print("│ Active rows {}, fixed cost {}, sub-problem LB {}, LB {}, UB {}\n", remaining_rows, subinst.get_fixed_cost(), local_LB,
                               global_LB, glo_UB_star);
                    fmt::print("│ Early exit: LB >= UB - 1\n");
                    fmt::print("└───────────────────────────────────────────────────────────────────────────────────\n\n");
                }
                break;
            }


            // 2. HEURISTIC PHASE
            auto& u_list = subgradient.explore(subgrad_UB, u_k, EXPLORING_ITERS);
            u_list.emplace_back(u_k);

            LocalSolution S_curr;
            real_t S_curr_cost = REAL_MAX;

            for (LocalMultipliers& u : u_list) {

                LocalSolution S = greedy(u);

                IF_DEBUG {
                    MStar cov;
                    cov.reset_covered(subinst.get_cols(), S, subinst.get_nrows());
                    assert(cov.get_uncovered() == 0);
                }

                real_t S_cost = S.compute_cost(subinst);
                real_t gS_cost = subinst.get_fixed_cost() + S_cost;
                subinst.update_sol_costs(S, gS_cost);

                if (S_cost < S_curr_cost) {

                    S_curr = S;
                    S_curr_cost = S_cost;
                    u_star = u;
                    if (gS_cost < glo_UB_star) {
                        glo_UB_star = gS_cost;
                        S_star = GlobalSolution(subinst, S);
                        IF_VERBOSE {
                            fmt::print("│ ══> [{:3}] Improved global UB: {} (fixed {} + local-cost {})\n", &u - &u_list[0], S_star.get_cost(),
                                       subinst.get_fixed_cost(), S_cost);
                        }
                    } else {
                        IF_VERBOSE {
                            fmt::print("│ ──> [{:3}] Improved local UB: {} (global value {}, best is {})\n", &u - &u_list[0], S_cost,
                                       GlobalSolution(subinst, S).get_cost(), glo_UB_star);
                        }
                    }
                }
            }

            if (subgradient.get_best_LB() > local_LB) {
                local_LB = subgradient.get_best_LB();  // u_k.compute_lb(subinst);
                global_LB = subinst.get_fixed_cost() + local_LB;
                if (iter == 1) { glo_u = GlobalMultipliers(subinst, u_k); }
            }

            fmt::print("│ Active rows {}, fixed cost {}, sub-problem LB {}, LB {}, UB {}\n", remaining_rows, subinst.get_fixed_cost(), local_LB, global_LB,
                       glo_UB_star);

            if (global_LB > glo_UB_star - HAS_INTEGRAL_COSTS) {
                IF_VERBOSE {
                    fmt::print("│ Early exit: LB >= UB - 1\n");
                    fmt::print("└───────────────────────────────────────────────────────────────────────────────────\n\n");
                }
                break;
            }

            IF_DEBUG {
                MStar cov;
                cov.reset_covered(subinst.get_cols(), S_curr, subinst.get_nrows());
                assert(cov.get_uncovered() == 0);
            }

            // 3. COLUMN FIXING
            auto glo_S_curr = GlobalSolution(subinst, S_curr);

            IF_DEBUG {
                MStar cov;
                cov.reset_covered(subinst.get_instance().get_cols(), glo_S_curr, subinst.get_instance().get_nrows());
                assert(cov.get_uncovered() == 0);
            }

            remaining_rows = col_fixing(u_star, glo_S_curr);

            u_k = SubGradient::u_perturbed_init(u_star, rnd);
            ++iter;

            IF_VERBOSE { fmt::print("└───────────────────────────────────────────────────────────────────────────────────\n\n"); }

        } while (remaining_rows > 0);

        return std::make_pair(S_star, glo_u);
    }

private:
    SubInstance& subinst;
    MStar& covered_rows;

    SubGradient subgradient;
    Greedy greedy;
    ColumnFixing col_fixing;

    std::mt19937& rnd;
};


#endif