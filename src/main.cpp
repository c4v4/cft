#include <fmt/core.h>
#include <fmt/ranges.h>

#include <cstdio>

#include "algorithms/Refinement.hpp"
#include "core/CliArgs.hpp"
#include "core/cft.hpp"
#include "instance/Instance.hpp"
#include "instance/parsing.hpp"

void print_inst_summary(cft::FileData const& fdata) {
    fmt::print("CFT  > Instance summary:\n");
    fmt::print("CFT  >   nrows:     {}\n", fdata.inst.rows.size());
    fmt::print("CFT  >   ncols:     {}\n", fdata.inst.cols.size());
    fmt::print("CFT  >   costs:     {} {} {} {} ...\n",
               fdata.inst.costs[0],
               fdata.inst.costs[1],
               fdata.inst.costs[2],
               fdata.inst.costs[3]);
    fmt::print("CFT  >   solcosts:  {} {} {} {} ...\n",
               fdata.inst.solcosts[0],
               fdata.inst.solcosts[1],
               fdata.inst.solcosts[2],
               fdata.inst.solcosts[3]);
    if (!fdata.warmstart.empty())
        fmt::print("CFT  >   warmstart: {} {} {} {} ...\n",
                   fdata.warmstart[0],
                   fdata.warmstart[1],
                   fdata.warmstart[2],
                   fdata.warmstart[3]);

    // print first 10 columns
    for (size_t i = 0; i < 4; ++i)
        fmt::print("CFT  >   col[{}]: {}\n", i, fmt::join(fdata.inst.cols[i], ", "));
}

int main(int argc, char const** argv) {
    fmt::print("CFT implementation by Luca Accorsi and Francesco Cavaliere.\n");
    fmt::print("Compiled on " __DATE__ " at " __TIME__ ".\n\n");

    try {
        auto timer    = cft::Chrono<>();
        auto cli_args = cft::parse_cli_args(argc, argv);
        cft::print_arg_values(cli_args);

        auto inst      = cft::Instance{};
        auto warmstart = cft::Solution{};

        if (cli_args.parser == CFT_RAIL_PARSER) {
            fmt::print("CFT  > Parsing RAIL instance from {}\n", cli_args.inst_path);
            inst = cft::parse_rail_instance(cli_args.inst_path);

        } else if (cli_args.parser == CFT_SCP_PARSER) {
            fmt::print("CFT  > Parsing SCP instance from {}\n", cli_args.inst_path);
            inst = cft::parse_scp_instance(cli_args.inst_path);

        } else if (cli_args.parser == CFT_CVRP_PARSER) {
            fmt::print("CFT  > Parsing CVRP instance from {}\n", cli_args.inst_path);
            auto file_data = cft::parse_cvrp_instance(cli_args.inst_path);
            inst           = std::move(file_data.inst);
            warmstart.idxs = std::move(file_data.warmstart);
            warmstart.cost = 0.0;
            for (cft::cidx_t j : warmstart.idxs)
                warmstart.cost += inst.costs[j];

        } else {
            fmt::print("CFT  > Parser {} does not exists.\n", cli_args.parser);
            cft::print_cli_help_msg();
            throw std::runtime_error("Parser does not exists.");
        }

        IF_DEBUG(print_inst_summary(inst));
        auto tlim = cli_args.time_limit;
        auto rnd  = cft::prng_t{cli_args.seed};
        auto sol  = cft::run(inst, rnd, tlim, warmstart);
        fmt::print("CFT  > Best solution {:.2f} time {:.2}\n", sol.cost, timer.elapsed<cft::sec>());

    } catch (std::exception const& e) {
        fmt::print("\nCFT  > ERROR: {}\n", e.what());
        std::fflush(stdout);
        std::fflush(stderr);
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
