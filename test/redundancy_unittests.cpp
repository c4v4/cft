#include <catch2/catch.hpp>

#include "greedy/redundancy.hpp"
#include "test_utils.hpp"
#include "utils/limits.hpp"
#include "utils/random.hpp"

namespace cft {

TEST_CASE("enumeration_removal removes redundant columns using implicit enumeration", "[cft]") {
    auto rnd  = prng_t(0);
    auto inst = make_easy_inst(0, 1000);
    for (int k = 0; k < 1000; ++k) {
        if ((k - 1) % 10 == 0)
            inst = make_easy_inst(k, 1000);

        cidx_t sol_size = roll_dice(rnd, 0, min(csize(inst.cols) - 1_C, 200_C));
        auto   sol      = Solution();
        for (int n = 0; n < sol_size; ++n) {
            cidx_t j = roll_dice(rnd, 0, csize(inst.cols) - 1_C);
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
        REQUIRE_NOTHROW(complete_init_redund_set(red_set, inst, sol.idxs, limits<real_t>::max()));
        CFT_IF_DEBUG(check_redundancy_data(inst, sol.idxs, red_set));

        REQUIRE_NOTHROW(heuristic_removal(red_set, inst));
        CFT_IF_DEBUG(check_redundancy_data(inst, sol.idxs, red_set));

        if (red_set.partial_cov_count < rsize(inst.rows))
            REQUIRE_NOTHROW(enumeration_removal(red_set, inst));

        remove_if(sol.idxs, [&](cidx_t j) {
            return any(red_set.cols_to_remove, [j](cidx_t r) { return r == j; });
        });

        for (ridx_t i = 0; i < rsize(inst.rows); ++i)
            REQUIRE((init_cov[i] >= red_set.total_cover[i]));
    }
}


}  // namespace cft
