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
#include "utils/random.hpp"

namespace cft {
namespace local { namespace {
    Instance make_easy_inst(uint64_t seed) {

        auto cols = SparseBinMat<ridx_t>();
        cols.push_back({1, 2, 3, 4, 5, 6, 7, 8, 9, 10});
        cols.push_back({11, 12, 13, 14, 15, 16, 17, 18, 19, 20});
        cols.push_back({21, 22, 23, 24, 25, 26, 27, 28, 29, 0});
        auto costs    = std::vector<real_t>{100, 100, 100};
        auto solcosts = std::vector<real_t>{300, 300, 300};

        auto rows = std::vector<ridx_t>{0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14,
                                        15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29};
        auto rnd  = prng_t{seed};
        int  num_cols = roll_dice(rnd, 0, 1000);
        for (int i = 0; i < num_cols; ++i) {
            std::shuffle(rows.begin(), rows.end(), rnd);
            size_t c_size = rnd() % 10;
            cols.push_back(make_span(rows.data(), c_size));
            costs.push_back(checked_cast<real_t>(roll_dice(rnd, 1, 3)));
            solcosts.push_back(1000);
        }

        auto inst     = Instance();
        inst.cols     = std::move(cols);
        inst.costs    = std::move(costs);
        inst.solcosts = std::move(solcosts);
        ridx_t nrows  = 30;
        fill_rows_from_cols(inst.cols, nrows, inst.rows);
        return inst;
    }
}  // namespace
}  // namespace local

TEST_CASE("Generate Instances", "[instances]") {
    auto env       = Environment();
    env.time_limit = 10.0;
    env.heur_iters = 10;
    env.verbose    = 0;

    for (int i = 0; i < 1000; ++i) {
        auto inst = local::make_easy_inst(i);
        auto sol  = run(env, inst);
        REQUIRE(sol.cost <= 300);
        if (sol.cost == 300)
            REQUIRE_THAT(sol.idxs, Catch::Matchers::UnorderedEquals(std::vector<cidx_t>{0, 1, 2}));
        CFT_IF_DEBUG(REQUIRE_NOTHROW(check_solution(inst, sol)));
    }
}

}  // namespace cft