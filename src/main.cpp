#include <fmt/core.h>
#include <fmt/ranges.h>

#include <algorithm>

#include "core/cft.hpp"
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

int main(int argc, char const** argv) {
    auto args = cft::make_span(argv, argc);

    print_inst_summary(cft::parse_scp_instance(args[1]));
    // print_inst_summary(cft::parse_rail_instance(args[2]));
    // print_inst_summary(cft::parse_cvrp_instance(args[3]));

    cft::Instance inst = cft::make_instance(cft::parse_scp_instance(args[1]));

    cft::real_t upper_bound = 1000;
    cft::prng_t rnd(0);
    auto        opt_res = cft::optimize(inst, upper_bound, cft::compute_greedy_multipliers(inst));
    auto        exp_res = cft::explore(inst,
                                upper_bound,
                                cft::compute_perturbed_multipliers(opt_res.lagr_mult, rnd));

    auto greedy = cft::make_greedy();
    for (auto& u_k : exp_res.lagr_mult_list) {
        auto        inout_sol = std::vector<cft::cidx_t>{};
        cft::real_t sol_cost  = greedy(inst, u_k, inout_sol);
        fmt::print("Greedy solution cost: {} (sol: {})\n", sol_cost, inout_sol);
    }

    return EXIT_SUCCESS;
}
