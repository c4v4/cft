#include <fmt/core.h>
#include <fmt/ranges.h>

#include "core/cft.hpp"
#include "fixing/ColFixing.hpp"
#include "greedy/Greedy.hpp"
#include "instance/Instance.hpp"
#include "instance/parsing.hpp"
#include "subgradient/subgradient.hpp"

void print_inst_summary(cft::InstanceData const& inst) {
    fmt::print("Instance summary:\n");
    fmt::print("  nrows:     {}\n", inst.nrows);
    fmt::print("  ncols:     {}\n", inst.cols.size());
    fmt::print("  costs:     {} {} {} {} ...\n",
               inst.costs[0],
               inst.costs[1],
               inst.costs[2],
               inst.costs[3]);
    fmt::print("  solcosts:  {} {} {} {} ...\n",
               inst.solcosts[0],
               inst.solcosts[1],
               inst.solcosts[2],
               inst.solcosts[3]);
    if (!inst.warmstart.empty())
        fmt::print("  warmstart: {} {} {} {} ...\n",
                   inst.warmstart[0],
                   inst.warmstart[1],
                   inst.warmstart[2],
                   inst.warmstart[3]);

    // print first 10 columns
    for (size_t i = 0; i < 4; ++i)
        fmt::print("  col[{}]: {}\n", i, fmt::join(inst.cols[i], ", "));
}

#ifndef NDEBUG
void check_solution(cft::Instance const&            inst,
                    std::vector<cft::cidx_t> const& sol,
                    cft::real_t                     sol_cost) {
    cft::ridx_t nrows = inst.rows.size();

    // check coverage
    cft::ridx_t covered_rows = 0;
    auto        cover_bits   = cft::make_cover_bits(nrows);
    for (auto j : sol)
        covered_rows += cover_bits.cover(inst.cols[j]);
    assert(covered_rows == nrows);

    // check cost
    cft::real_t total_cost = 0;
    for (cft::cidx_t j : sol)
        total_cost += inst.costs[j];
    assert(std::abs(total_cost - sol_cost) < 1e-6);
}
#endif

int main(int argc, char const** argv) {
    auto args = cft::make_span(argv, argc);

    constexpr int min_row_coverage = 5;
    cft::Instance inst             = cft::make_instance(cft::parse_rail_instance(args[1]));

    auto greedy     = cft::make_greedy();
    auto col_fixing = cft::make_col_fixing();
    auto rnd        = cft::prng_t{0};

    auto best_sol  = std::vector<cft::cidx_t>{};
    auto best_cost = cft::limits<cft::real_t>::max();

    while (!inst.rows.empty()) {
        auto core_inst        = cft::make_tentative_core_instance(inst, min_row_coverage);
        auto better_sol       = std::vector<cft::cidx_t>{};
        auto better_lagr_mult = cft::compute_greedy_multipliers(core_inst);
        auto better_cost      = greedy(core_inst, better_lagr_mult, better_sol);

        auto opt_res = cft::optimize(inst,
                                     core_inst,
                                     better_cost,  // TODO(any): best_cost - fixed_cost?
                                     cft::compute_greedy_multipliers(core_inst));
        auto exp_res = cft::explore(core_inst,
                                    better_cost,  // TODO(any): best_cost - fixed_cost?
                                    cft::compute_perturbed_multipliers(opt_res.lagr_mult, rnd));

        for (size_t l = 0; l < exp_res.lagr_mult_list.size(); ++l) {
            auto&       lagr_mult = exp_res.lagr_mult_list[l];
            auto        inout_sol = std::vector<cft::cidx_t>{};
            cft::real_t sol_cost  = greedy(inst, lagr_mult, inout_sol, best_cost);

            if (sol_cost < better_cost) {
                better_cost      = sol_cost;
                better_sol       = inout_sol;
                better_lagr_mult = lagr_mult;
                IF_DEBUG(check_solution(inst, best_sol, best_cost));
            }
            if (better_cost < best_cost) {
                best_cost = better_cost;
                best_sol  = better_sol;
            }

            fmt::print("Greedy solution cost: {}, better: {}, best: {}\n",
                       sol_cost,
                       better_cost,
                       best_cost);
        }

        // TODO(cava): Col fixing for inst considering core-inst?
        col_fixing(inst, better_lagr_mult, best_sol, greedy);
        fmt::print("Remaining rows after column fixing: {}\n", inst.rows.size());
    }

    return EXIT_SUCCESS;
}
