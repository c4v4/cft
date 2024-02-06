#include <fmt/core.h>
#include <fmt/ranges.h>

#include <algorithm>

#include "parsing.hpp"

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
    print_inst_summary(cft::parse_rail_instance(args[2]));
    print_inst_summary(cft::parse_cvrp_instance(args[3]));

    return EXIT_SUCCESS;
}
