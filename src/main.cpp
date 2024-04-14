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

#include <cstdio>

#include "algorithms/Refinement.hpp"
#include "core/CliArgs.hpp"
#include "core/Instance.hpp"
#include "core/cft.hpp"
#include "core/parsing.hpp"
#include "utils/print.hpp"

namespace local { namespace {
#ifndef NDEBUG
    void print_inst_summary(cft::Environment const& env,
                            cft::Instance const&    inst,
                            cft::Solution const&    warmstart = {}) {
        if (env.verbose < 5)
            return;

        cft::print<5>(env, "CFT > Instance summary:\n");
        cft::print<5>(env, "CFT >   nrows:     {}\n", cft::rsize(inst.rows));
        cft::print<5>(env, "CFT >   ncols:     {}\n", cft::csize(inst.cols));
        cft::print<5>(env,
                      "CFT >   costs:     {} {} {} {} ...\n",
                      inst.costs[0],
                      inst.costs[1],
                      inst.costs[2],
                      inst.costs[3]);
        if (!warmstart.idxs.empty())
            cft::print<5>(env,
                          "CFT >   warmstart: {} {} {} {} ...\n",
                          warmstart.idxs[0],
                          warmstart.idxs[1],
                          warmstart.idxs[2],
                          warmstart.idxs[3]);

        // print first 10 columns
        for (size_t i = 0; i < 4; ++i)
            cft::print<5>(env, "CFT >   col[{}]: {}\n", i, inst.cols[i]);
    }
#endif

    cft::FileData parse_inst_and_initsol(cft::Environment const& env) {
        auto fdata = cft::FileData();

        if (env.parser == CFT_RAIL_PARSER) {
            cft::print<1>(env, "CFT > Parsing RAIL instance from {}\n", env.inst_path);
            fdata.inst = cft::parse_rail_instance(env.inst_path);

        } else if (env.parser == CFT_SCP_PARSER) {
            cft::print<1>(env, "CFT > Parsing SCP instance from {}\n", env.inst_path);
            fdata.inst = cft::parse_scp_instance(env.inst_path);

        } else if (env.parser == CFT_CVRP_PARSER) {
            cft::print<1>(env, "CFT > Parsing CVRP instance from {}\n", env.inst_path);
            fdata = cft::parse_cvrp_instance(env.inst_path);

        } else if (env.parser == CFT_MPS_PARSER) {
            cft::print<1>(env, "CFT > Parsing MPS instance from {}\n", env.inst_path);
            fdata.inst = cft::parse_mps_instance(env.inst_path);

        } else {
            cft::print<1>(env, "CFT > Parser {} does not exists.\n", env.parser);
            cft::print_cli_help_msg();
            throw std::runtime_error("Parser does not exists.");
        }

        if (!env.initsol_path.empty()) {
            fdata.init_sol = cft::parse_solution(env.initsol_path);
            CFT_IF_DEBUG(check_solution(fdata.inst, fdata.init_sol));
        }

        CFT_IF_DEBUG(print_inst_summary(env, fdata.inst, fdata.init_sol));

        return fdata;
    }

    inline std::string make_sol_name(std::string const& inst_path) {
        auto out_name = cft::StringView(inst_path);

        // Remove path if present
        auto slash_pos = out_name.find_last_if([](char c) { return c == '/' || c == '\\'; });
        if (slash_pos != out_name.size())
            out_name = out_name.get_substr(slash_pos + 1, out_name.size());

        // Remove extension
        out_name = out_name.get_substr(0, out_name.find_last_if([](char c) { return c == '.'; }));

        assert(!out_name.empty());
        return out_name.to_cpp_string() += ".sol";
    }
}  // namespace
}  // namespace local

int main(int argc, char const** argv) {

    try {
        auto env = cft::parse_cli_args(argc, argv);

        cft::print<1>(env, "CFT implementation by Luca Accorsi and Francesco Cavaliere.\n");
        cft::print<1>(env, "Compiled on " __DATE__ " at " __TIME__ ".\n\n");

        if (env.inst_path.empty())
            throw std::runtime_error("Instance file path not provided.");

        if (env.sol_path.empty())
            env.sol_path = local::make_sol_name(env.inst_path);

        cft::print_arg_values(env);

        auto fdata = local::parse_inst_and_initsol(env);
        auto sol   = cft::run(env, fdata.inst, fdata.init_sol);
        write_solution(env.sol_path, sol);
        cft::print<1>(env,
                      "CFT > Best solution {:.2f} time {:.2f}\n",
                      sol.cost,
                      env.timer.elapsed<cft::sec>());

    } catch (std::exception const& e) {
        fmt::print(stderr, "\nCFT  > ERROR: {}\n", e.what());
        std::fflush(stdout);
        std::fflush(stderr);
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
