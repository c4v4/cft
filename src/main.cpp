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

#include <fmt/core.h>
#include <fmt/ranges.h>

#include <cstdio>

#include "algorithms/Refinement.hpp"
#include "core/Instance.hpp"
#include "core/cft.hpp"
#include "core/parsing.hpp"
#include "utils/CliArgs.hpp"

void print_inst_summary(cft::Instance const& inst, cft::Solution const& warmstart = {}) {
    fmt::print("CFT > Instance summary:\n");
    fmt::print("CFT >   nrows:     {}\n", cft::rsize(inst.rows));
    fmt::print("CFT >   ncols:     {}\n", cft::csize(inst.cols));
    fmt::print("CFT >   costs:     {} {} {} {} ...\n",
               inst.costs[0],
               inst.costs[1],
               inst.costs[2],
               inst.costs[3]);
    fmt::print("CFT >   solcosts:  {} {} {} {} ...\n",
               inst.solcosts[0],
               inst.solcosts[1],
               inst.solcosts[2],
               inst.solcosts[3]);
    if (!warmstart.idxs.empty())
        fmt::print("CFT >   warmstart: {} {} {} {} ...\n",
                   warmstart.idxs[0],
                   warmstart.idxs[1],
                   warmstart.idxs[2],
                   warmstart.idxs[3]);

    // print first 10 columns
    for (size_t i = 0; i < 4; ++i)
        fmt::print("CFT >   col[{}]: {}\n", i, fmt::join(inst.cols[i], ", "));
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
            fmt::print("CFT > Parsing RAIL instance from {}\n", cli_args.inst_path);
            inst = cft::parse_rail_instance(cli_args.inst_path);

        } else if (cli_args.parser == CFT_SCP_PARSER) {
            fmt::print("CFT > Parsing SCP instance from {}\n", cli_args.inst_path);
            inst = cft::parse_scp_instance(cli_args.inst_path);

        } else if (cli_args.parser == CFT_CVRP_PARSER) {
            fmt::print("CFT > Parsing CVRP instance from {}\n", cli_args.inst_path);
            auto file_data = cft::parse_cvrp_instance(cli_args.inst_path);
            inst           = std::move(file_data.inst);
            warmstart.idxs = std::move(file_data.warmstart);
            warmstart.cost = 0.0;
            for (cft::cidx_t j : warmstart.idxs)
                warmstart.cost += inst.costs[j];

        } else if (cli_args.parser == CFT_MPS_PARSER) {
            fmt::print("CFT > Parsing MPS instance from {}\n", cli_args.inst_path);
            inst = cft::parse_mps_instance(cli_args.inst_path);

        } else {
            fmt::print("CFT > Parser {} does not exists.\n", cli_args.parser);
            cft::print_cli_help_msg();
            throw std::runtime_error("Parser does not exists.");
        }

        CFT_IF_DEBUG(print_inst_summary(inst, warmstart));
        auto tlim = cli_args.time_limit;
        auto rnd  = cft::prng_t{cli_args.seed};
        auto sol  = cft::run(inst, rnd, tlim, warmstart);
        fmt::print("CFT > Best solution {:.2f} time {:.2f}\n", sol.cost, timer.elapsed<cft::sec>());

    } catch (std::exception const& e) {
        fmt::print("\nCFT  > ERROR: {}\n", e.what());
        std::fflush(stdout);
        std::fflush(stderr);
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
