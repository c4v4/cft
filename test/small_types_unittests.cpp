// SPDX-FileCopyrightText: 2024 Francesco Cavaliere <francescocava95@gmail.com>
// SPDX-License-Identifier: MIT

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#define DOCTEST_CONFIG_SUPER_FAST_ASSERTS
#include <doctest/doctest.h>

#define CFT_CIDX_TYPE int16_t
#define CFT_RIDX_TYPE uint8_t
#define CFT_REAL_TYPE float

#include "algorithms/Refinement.hpp"
#include "core/cft.hpp"
#include "test_utils.hpp"

namespace cft {

TEST_CASE("Custom types, whole algorithm run test") {
    auto env       = Environment();
    env.time_limit = 10.0;
    env.heur_iters = 100;
    env.verbose    = 2;
    auto init_sol  = Solution();
    init_sol.idxs  = std::vector<cidx_t>{0_C, 1_C, 2_C, 3_C, 4_C, 5_C, 6_C, 7_C, 8_C, 9_C};
    init_sol.cost  = 1000.0_F;

    for (int n = 0; n < 20; ++n) {
        auto inst = Instance();
        auto res  = CftResult();
        REQUIRE_NOTHROW(inst = make_easy_inst(n, 1000_C));
        REQUIRE_NOTHROW(res = run(env, inst, init_sol));
        CHECK((res.sol.cost <= 1000.0_F));  // Trivial bad solution has 1000 cost
        CHECK((res.sol.cost >= as_real(res.sol.idxs.size())));  // Min col cost is 1.0
        if ((abs(res.sol.cost - 1000.0_F) < 1e-6_F))
            CHECK(res.sol.idxs == init_sol.idxs);
        CFT_IF_DEBUG(CHECK_NOTHROW(check_inst_solution(inst, res.sol)));
    }
}

}  // namespace cft
