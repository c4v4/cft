// SPDX-FileCopyrightText: 2024 Francesco Cavaliere <francescocava95@gmail.com>
// SPDX-License-Identifier: MIT

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#define DOCTEST_CONFIG_SUPER_FAST_ASSERTS
 
#include <doctest/doctest.h>

#include "greedy/redundancy.hpp"
#include "test_utils.hpp"
#include "utils/limits.hpp"
#include "utils/random.hpp"

namespace cft {

TEST_CASE("enumeration_removal removes redundant columns using implicit enumeration") {
    auto rnd  = prng_t(0);
    auto inst = make_easy_inst(0, 1000_C);
    for (int k = 0; k < 1000; ++k) {
        if ((k - 1) % 10 == 0)
            inst = make_easy_inst(k, 1000_C);

        cidx_t sol_size = roll_dice(rnd, 0_C, min(csize(inst.cols) - 1_C, 200_C));
        auto   sol      = Solution();
        for (cidx_t n = 0_C; n < sol_size; ++n) {
            cidx_t j = roll_dice(rnd, 0_C, csize(inst.cols) - 1_C);
            if (!any(sol.idxs, [j](cidx_t sj) { return sj == j; }))
                sol.idxs.push_back(j);
        }

        auto red_set = RedundancyData();
        red_set.total_cover.reset(100);
        for (cidx_t j : sol.idxs) {
            sol.cost += inst.costs[j];
            red_set.redund_set.push_back({j, inst.costs[j]});
            red_set.total_cover.cover(inst.cols[j]);
        }

        auto init_cov = red_set.total_cover;
        CHECK_NOTHROW(complete_init_redund_set(inst, sol.idxs, limits<real_t>::max(), red_set));
        CFT_IF_DEBUG(check_redundancy_data(inst, sol.idxs, red_set));

        CHECK_NOTHROW(heuristic_removal(inst, red_set));
        CFT_IF_DEBUG(check_redundancy_data(inst, sol.idxs, red_set));

        if (red_set.partial_cov_count < rsize(inst.rows))
            CHECK_NOTHROW(enumeration_removal(inst, red_set));

        remove_if(sol.idxs, [&](cidx_t j) {
            return any(red_set.cols_to_remove, [j](cidx_t r) { return r == j; });
        });

        for (ridx_t i = 0_R; i < rsize(inst.rows); ++i)
            CHECK((init_cov[i] >= red_set.total_cover[i]));
    }
}


}  // namespace cft
