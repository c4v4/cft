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
#include "test_utils.hpp"

TEST_CASE("Define dummy Instance") {
    REQUIRE_NOTHROW([] {
        auto inst = cft::Instance();

        // Insert the columns one at a time
        inst.cols.push_back({0, 1, 2, 3});
        inst.cols.push_back({1, 3, 0, 4});
        inst.cols.push_back({4, 1, 2});

        // Define columns costs (Note, every column must have a cost)
        inst.costs = {1.0, 2.0, 3.0};

        // Rows can be automatically filled from columns using this helper function
        cft::fill_rows_from_cols(inst.cols, 5, inst.rows);

        // Check that the defined instance is well formed (costly operation)
        CFT_IF_DEBUG(cft::col_and_rows_check(inst.cols, inst.rows));
    }());
}

TEST_CASE("Invoke whole algorithm") {
    // Setup
    auto inst = cft::Instance();
    REQUIRE_NOTHROW(inst = cft::make_easy_inst(10, 1000));

    // Test Readme example
    REQUIRE_NOTHROW([&] {
        auto env       = cft::Environment();
        env.time_limit = 10.0;  // Time limit in seconds
        env.verbose    = 2;     // Log verbosity level (from 0 to 5)
        env.timer.restart();    // NOTE: the timer is used also for the time limit test
        auto res = cft::run(env, inst);
        fmt::print("CFT solution cost: {}\n", res.sol.cost);
    }());
}

TEST_CASE("Invoke 3-phase algorithm") {
    // Setup
    auto env       = cft::Environment();
    env.time_limit = 10.0;
    env.verbose    = 1;

    auto inst = cft::Instance();
    REQUIRE_NOTHROW(inst = cft::make_easy_inst(10, 1000));

    // Test Readme example
    REQUIRE_NOTHROW([&] {
        auto three_phase = cft::ThreePhase();
        auto result      = three_phase(env, inst);
        fmt::print("3-phase solution cost: {}, LB: {}\n", result.sol.cost, result.dual.lb);
    }());
}

TEST_CASE("Invoke greedy algorithm") {
    // Setup
    auto env       = cft::Environment();
    env.time_limit = 10.0;
    env.verbose    = 1;

    auto inst = cft::Instance();
    REQUIRE_NOTHROW(inst = cft::make_easy_inst(10, 1000));

    // Test Readme example
    REQUIRE_NOTHROW([&] {
        auto num_rows      = cft::rsize(inst.rows);
        auto lagr_mult     = std::vector<cft::real_t>(num_rows, 0.0);
        auto reduced_costs = inst.costs;  // Zero multipliers implies red-costs = costs

        auto greedy = cft::Greedy();
        auto sol    = cft::Solution();
        sol.cost    = greedy(inst, lagr_mult, reduced_costs, sol.idxs);
        fmt::print("Greedy solution cost: {}\n", sol.cost);
    }());
}
