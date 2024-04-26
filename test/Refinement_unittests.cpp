// SPDX-FileCopyrightText: 2024 Francesco Cavaliere <francescocava95@gmail.com>
// SPDX-License-Identifier: MIT

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#define DOCTEST_CONFIG_SUPER_FAST_ASSERTS
#include <doctest/doctest.h>

#include "algorithms/Refinement.hpp"
#include "core/Instance.hpp"
#include "core/cft.hpp"
#include "test_utils.hpp"

namespace cft {

TEST_CASE("Whole algorithm run test") {
    auto env       = Environment();
    env.time_limit = 10.0;
    env.heur_iters = 100;
    env.verbose    = 1;
    auto init_sol  = Solution();
    init_sol.idxs  = std::vector<cidx_t>{0_C, 1_C, 2_C, 3_C, 4_C, 5_C, 6_C, 7_C, 8_C, 9_C};
    init_sol.cost  = 1000.0_F;

    for (int n = 0; n < 100; ++n) {
        auto inst = Instance();
        auto sol  = Solution();
        REQUIRE_NOTHROW(inst = make_easy_inst(n, 1000_C));
        REQUIRE_NOTHROW(sol = run(env, inst, init_sol));
        CHECK(sol.cost <= 1000.0_F);                  // Trivial bad solution has 1000 cost
        CHECK(sol.cost >= as_real(sol.idxs.size()));  // Min col cost is 1.0
        if (abs(sol.cost - 1000.0_F) < 1e-6_F)
            CHECK(sol.idxs == init_sol.idxs);
        CFT_IF_DEBUG(CHECK_NOTHROW(check_inst_solution(inst, sol)));
    }
}

TEST_CASE("from_fixed_to_unfixed_sol test") {
    // Test case 1
    auto sol1                 = Solution();
    sol1.idxs                 = std::vector<cidx_t>{0_C, 1_C, 2_C, 3_C};
    sol1.cost                 = 10.0_F;
    auto fixing1              = FixingData();
    fixing1.fixed_cost        = 5.0_F;
    fixing1.fixed_cols        = std::vector<cidx_t>{0_C, 1_C, 2_C, 3_C};
    fixing1.curr2orig.col_map = std::vector<cidx_t>{4_C, 5_C, 6_C, 7_C};
    auto best_sol1            = Solution();
    local::from_fixed_to_unfixed_sol(sol1, fixing1, best_sol1);
    CHECK(best_sol1.cost == 15);
    CHECK(best_sol1.idxs == std::vector<cidx_t>{0_C, 1_C, 2_C, 3_C, 4_C, 5_C, 6_C, 7_C});
}

}  // namespace cft