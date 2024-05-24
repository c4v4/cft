// SPDX-FileCopyrightText: 2024 Francesco Cavaliere <francescocava95@gmail.com>
// SPDX-License-Identifier: MIT

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#define DOCTEST_CONFIG_SUPER_FAST_ASSERTS

#include <doctest/doctest.h>

#define CFT_CIDX_TYPE int
#define CFT_RIDX_TYPE int
#define CFT_REAL_TYPE float

#include "algorithms/Refinement.hpp"
#include "algorithms/ThreePhase.hpp"
#include "core/Instance.hpp"
#include "core/cft.hpp"
#include "greedy/Greedy.hpp"
#include "subgradient/Subgradient.hpp"
#include "test_utils.hpp"

TEST_CASE("Define dummy Instance") {
    auto inst = cft::Instance();

    // Insert the columns one at a time
    inst.cols.push_back({0, 1, 2, 3});
    inst.cols.push_back({1, 3, 0, 4});
    inst.cols.push_back({4, 1, 2});

    // Define columns costs (Note, every column must have a cost)
    inst.costs = {1.0, 2.0, 3.0};

    // Rows can be automatically computed from columns using this helper function
    CHECK_NOTHROW(cft::fill_rows_from_cols(inst.cols, cft::rsize(inst.rows), inst.rows));

    // Check that the defined instance is well formed (costly operation)
    CFT_IF_DEBUG(CHECK_NOTHROW(cft::col_and_rows_check(inst.cols, inst.rows)));
}

TEST_CASE("Invoke whole algorithm") {
    // Setup
    auto env       = cft::Environment();
    env.time_limit = 10.0;
    env.verbose    = 1;
    env.heur_iters = 100;

    auto inst = cft::Instance();
    auto sol  = cft::Solution();
    REQUIRE_NOTHROW(inst = cft::make_easy_inst(10, 1000));

    // Test Readme example
    REQUIRE_NOTHROW(sol = cft::run(env, inst));

    CHECK(sol.cost <= 1000.0);
    CHECK(sol.cost >= cft::as_real(cft::size(sol.idxs)));
}

TEST_CASE("Invoke 3-phase algorithm") {
    // Setup
    auto env       = cft::Environment();
    env.time_limit = 10.0;
    env.heur_iters = 100;
    env.verbose    = 1;

    auto inst = cft::Instance();
    REQUIRE_NOTHROW(inst = cft::make_easy_inst(10, 1000));

    // Test Readme example
    auto three_phase = cft::ThreePhase();
    auto result_3p   = cft::ThreePhaseResult();
    REQUIRE_NOTHROW(result_3p = three_phase(env, inst));
    fmt::print("3-phase solution cost: {}, LB: {}\n", result_3p.sol.cost, result_3p.nofix_lb);
}

TEST_CASE("Invoke greedy algorithm") {
    // Setup
    auto env       = cft::Environment();
    env.time_limit = 10.0;
    env.heur_iters = 100;
    env.verbose    = 1;

    auto inst = cft::Instance();
    REQUIRE_NOTHROW(inst = cft::make_easy_inst(10, 1000));

    // Test Readme example
    auto greedy        = cft::Greedy();
    auto sol           = cft::Solution();
    auto lagr_mult     = std::vector<cft::real_t>(cft::rsize(inst.rows), 0.0);
    auto reduced_costs = inst.costs;  // multipliers = 0  =>  red-costs = costs
    REQUIRE_NOTHROW(sol.cost = greedy(inst, lagr_mult, reduced_costs, sol.idxs));
    fmt::print("Greedy solution cost: {}\n", sol.cost);
}
