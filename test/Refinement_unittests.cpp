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


#include <catch2/catch.hpp>

#include "algorithms/Refinement.hpp"
#include "core/Instance.hpp"
#include "core/cft.hpp"
#include "test_utils.hpp"

namespace cft {

TEST_CASE("Whole algorithm run test", "[Refinement]") {
    auto env       = Environment();
    env.time_limit = 10.0;
    env.heur_iters = 100;
    env.verbose    = 1;
    auto init_sol  = Solution();
    init_sol.idxs  = std::vector<cidx_t>{0_C, 1_C, 2_C, 3_C, 4_C, 5_C, 6_C, 7_C, 8_C, 9_C};
    init_sol.cost  = 1000.0_F;

    SECTION("Test with easy instances") {
        for (int n = 0; n < 100; ++n) {
            auto inst = make_easy_inst(n, 1000);
            auto sol  = run(env, inst, init_sol);
            REQUIRE(sol.cost <= 1000.0_F);
            if (abs(sol.cost - 1000.0_F) < 1e-6_F)
                REQUIRE_THAT(sol.idxs, Catch::Matchers::UnorderedEquals(init_sol.idxs));
            CFT_IF_DEBUG(REQUIRE_NOTHROW(check_solution(inst, sol)));
        }
    }
}

TEST_CASE("from_fixed_to_unfixed_sol test", "[Refinement]") {
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
    REQUIRE(best_sol1.cost == 15);
    REQUIRE(best_sol1.idxs == std::vector<cidx_t>{0_C, 1_C, 2_C, 3_C, 4_C, 5_C, 6_C, 7_C});
}

}  // namespace cft