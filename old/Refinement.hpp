#ifndef SCP_INCLUDE_REFINEMENT_HPP_
#define SCP_INCLUDE_REFINEMENT_HPP_

#include "Instance.hpp"
#include "ThreePhase.hpp"
#include "cft.hpp"

#define ALPHA  1.1f
#define BETA   1.0f
#define PI_MIN 0.3f

#define PI_MAX          0.9
#define POST_OPT_TRIALS 0  // 100

class Refinement {
public:
    Refinement(Instance& inst_, cft::prng_t& rnd_)
        : inst(inst_)
        , subinst(inst_)
        , three_phase(subinst, rnd_)
        , rnd(rnd_) {
    }

    inline void refinement_fix(GlobalSolution      S_star,
                               GlobalMultipliers   u_star,
                               real_t              pi,
                               std::vector<idx_t>& cols_to_fix,
                               CountSet&           covering_times) {
        Cols& cols = inst.get_cols();

        covering_times.reset_covered(cols, S_star, inst.get_nrows());

        deltas.resize(S_star.size());
        for (idx_t j = 0; j < S_star.size(); ++j) {
            auto& col        = cols[S_star[j]];
            deltas[j].first  = S_star[j];
            deltas[j].second = std::max<real_t>(col.compute_lagr_cost(u_star), 0.0);
            for (auto i : col)
                deltas[j].second += u_star[i] * (covering_times[i] - 1.0) / covering_times[i];
        }
        std::sort(deltas.begin(), deltas.end(), [](auto& a, auto& b) {
            return a.second < b.second;
        });

        covering_times.reset_uncovered(inst.get_nrows());
        real_t covered_fraction = 0.0;
        idx_t  n                = 0;
        for (; n < deltas.size() && covered_fraction < pi; ++n) {
            covering_times.cover_rows(cols[deltas[n].first]);
            covered_fraction = static_cast<real_t>(covering_times.get_covered()) /
                               static_cast<real_t>(inst.get_nrows());
        }

        cols_to_fix.resize(n);
        for (idx_t j2 = 0; j2 < n; ++j2)
            cols_to_fix[j2] = deltas[j2].first;
    }

    inline void random_fix(GlobalSolution      S_star,
                           real_t              pi,
                           std::vector<idx_t>& cols_to_fix,
                           CountSet&           covering_times) {
        Cols& cols = inst.get_cols();

        std::shuffle(S_star.begin(), S_star.end(), rnd);

        cols_to_fix.clear();
        covering_times.reset_uncovered(inst.get_nrows());
        real_t covered_fraction = 0.0;

        for (idx_t n = 0; n < S_star.size() && covered_fraction < pi; ++n) {
            cols_to_fix.emplace_back(S_star[n]);
            covering_times.cover_rows(cols[S_star[n]]);
            covered_fraction = static_cast<real_t>(covering_times.get_covered()) /
                               static_cast<real_t>(inst.get_nrows());
        }
    }

    inline void random_fix2(GlobalSolution      S_star,
                            GlobalMultipliers   u_star,
                            real_t              pi,
                            std::vector<idx_t>& cols_to_fix,
                            CountSet&           covering_times) {
        Cols&                                  cols = inst.get_cols();
        std::uniform_real_distribution<real_t> dist(0.5, 1.5);

        covering_times.reset_covered(cols, S_star, inst.get_nrows());

        deltas.resize(S_star.size());
        for (idx_t j = 0; j < S_star.size(); ++j) {
            auto& col        = cols[S_star[j]];
            deltas[j].first  = S_star[j];
            deltas[j].second = std::max<real_t>(col.compute_lagr_cost(u_star), 0.0);
            for (auto i : col)
                deltas[j].second += u_star[i] * (covering_times[i] - 1.0) / covering_times[i];
            deltas[j].second *= dist(rnd);
        }
        std::sort(deltas.begin(), deltas.end(), [](auto& a, auto& b) {
            return a.second < b.second;
        });

        covering_times.reset_uncovered(inst.get_nrows());
        real_t covered_fraction = 0.0;
        idx_t  n                = 0;
        for (; n < deltas.size() && covered_fraction < pi; ++n) {
            covering_times.cover_rows(cols[deltas[n].first]);
            covered_fraction = static_cast<real_t>(covering_times.get_covered()) /
                               static_cast<real_t>(inst.get_nrows());
        }

        cols_to_fix.resize(n);
        for (idx_t j2 = 0; j2 < n; ++j2)
            cols_to_fix[j2] = deltas[j2].first;
    }

    // S must be a global complete solution
    std::vector<idx_t> operator()([[maybe_unused]] std::vector<idx_t> const& S_init) {

        // 1.
        GlobalSolution     S_star;
        std::vector<idx_t> cols_to_fix;

        if (!S_init.empty()) {
            real_t cost = 0.0;
            for (idx_t j : S_init)
                cost += inst.get_col(j).get_cost();
            for (auto j : S_init)
                S_star.push_back(j);
            S_star.set_cost(cost);
            IF_VERBOSE {
                fmt::print("Found warm start with cost {}.\n", cost);
            }
        }

        GlobalMultipliers u_star;
        real_t            best_LB = REAL_LOWEST;

        real_t pi                = PI_MIN;
        real_t last_improving_pi = pi;

        int post_optimization_trials = POST_OPT_TRIALS;

        idx_t iter = 1;
        do {
            subinst.reset();  // 2.

            {
                auto [S, u] = three_phase(S_star.get_cost());  // 3. & 4.

                assert(std::fabs(pi - PI_MIN) <= 0.001 || !inst.get_fixed_cols().empty());
                pi *= ALPHA;  // 6.

                if (S.get_cost() < S_star.get_cost()) {  // update best solution
                    S_star = std::move(S);               // 5.

                    last_improving_pi = pi;

                    // pi = PI_MIN;            // 6.
                    pi      = std::max(pi / (ALPHA * ALPHA), PI_MIN);  // 6.
                    best_LB = REAL_LOWEST;
                }

                if (iter == 1)
                    u_star = std::move(u);

                if (u.get_lb() > best_LB) {
                    best_LB = u.get_lb();
                }  // ?. update best lower bound

                if (S_star.get_cost() - 1.0 <= BETA * best_LB || pi > PI_MAX ||
                    inst.get_active_rows_size() <= 0) {

                    if (post_optimization_trials <= 0) {
                        IF_VERBOSE {
                            fmt::print("╔═ REFINEMENT: iter {:2} ═════════════════════════════════"
                                       "════════════════════════════\n",
                                       iter);
                            fmt::print("║ Early Exit: β(={}) * LB(={}) > UB(={}) - 1\n",
                                       BETA,
                                       best_LB,
                                       S_star.get_cost());
                            fmt::print("║ Active rows {}, active cols {}, pi {}\n",
                                       inst.get_active_rows_size(),
                                       inst.get_active_cols().size(),
                                       pi);
                            fmt::print("║ LB {}, UB {}, UB size {}\n",
                                       best_LB,
                                       S_star.get_cost(),
                                       S_star.size());
                            fmt::print("╚══════════════════════════════════════════════════════"
                                       "═════════════════════════════\n\n");
                        }
                        break;
                    }

                    --post_optimization_trials;
                    fmt::print("   POST-OPTIMIZATION REFINEMENT: iter {:2}\n",
                               POST_OPT_TRIALS - post_optimization_trials);

                    pi                = last_improving_pi;
                    last_improving_pi = std::max(PI_MIN, last_improving_pi / ALPHA);
                    best_LB           = REAL_LOWEST;
                }

            }  //(S and u are potentially been moved, better to encapsulate them into a block)

            pi = std::min<real_t>(PI_MAX, pi);

            // 7. Refinement Fix
            inst.reset_fixing();

            CountSet& covering_times = subinst.get_covering_times();
            if (post_optimization_trials == POST_OPT_TRIALS) {
                refinement_fix(S_star, u_star, pi, cols_to_fix, covering_times);
            } else {
                // if (post_optimization_trials <= 0) { break; }
                //--post_optimization_trials;
                // cols_to_fix = random_fix2(S_star, u_star, pi);
                random_fix(S_star, pi, cols_to_fix, covering_times);
            }

            assert(cols_to_fix.size() <= S_star.size());

            std::sort(cols_to_fix.begin(), cols_to_fix.end());
            inst.fix_columns(cols_to_fix, covering_times);

            IF_VERBOSE {
                fmt::print("╔═ REFINEMENT: iter {:2} "
                           "═════════════════════════════════════════════════════════════\n",
                           iter);
                fmt::print("║ Active rows {}, active cols {}, pi {}\n",
                           inst.get_active_rows_size(),
                           inst.get_active_cols().size(),
                           pi);
                fmt::print("║ LB {}, UB {}, UB size {}\n",
                           best_LB,
                           S_star.get_cost(),
                           S_star.size());
                fmt::print("╚══════════════════════════════════════════════════════════════════"
                           "═════════════════\n\n");
            }

            assert(inst.get_fixed_cost() <= S_star.get_cost());
            ++iter;
        } while (true);

        return S_star;
    }

private:
    Instance& inst;

    SubInstance subinst;
    ThreePhase  three_phase;

    cft::prng_t& rnd;

    // retain allocated memory (anti-RAII, should be local)
    std::vector<std::pair<idx_t, real_t>> deltas;
};

#endif