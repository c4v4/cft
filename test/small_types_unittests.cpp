// SPDX-FileCopyrightText: 2024 Francesco Cavaliere <francescocava95@gmail.com>
// SPDX-License-Identifier: MIT

#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>

#define CFT_CIDX_TYPE int16_t
#define CFT_RIDX_TYPE uint8_t
#define CFT_REAL_TYPE float

#include "algorithms/Refinement.hpp"
#include "core/cft.hpp"
#include "test_utils.hpp"

namespace cft {

TEST_CASE("Custom types, whole algorithm run test", "[Refinement]") {
    auto env       = Environment();
    env.time_limit = 10.0;
    env.heur_iters = 100;
    env.verbose    = 2;
    auto init_sol  = Solution();
    init_sol.idxs  = std::vector<cidx_t>{0_C, 1_C, 2_C, 3_C, 4_C, 5_C, 6_C, 7_C, 8_C, 9_C};
    init_sol.cost  = 1000.0_F;

    SECTION("Test with easy instances") {
        for (int n = 0; n < 20; ++n) {
            auto inst = make_easy_inst(n, 100_C);
            auto sol  = run(env, inst, init_sol);
            REQUIRE((sol.cost <= 1000.0_F));                  // Trivial bad solution has 1000 cost
            REQUIRE((sol.cost >= as_real(sol.idxs.size())));  // Min col cost is 1.0
            if ((abs(sol.cost - 1000.0_F) < 1e-6_F))
                REQUIRE_THAT(sol.idxs, Catch::Matchers::UnorderedEquals(init_sol.idxs));
            CFT_IF_DEBUG(REQUIRE_NOTHROW(check_inst_solution(inst, sol)));
        }
    }
}

}  // namespace cft
