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

#ifndef CFT_SRC_CORE_CLIARGS_HPP
#define CFT_SRC_CORE_CLIARGS_HPP


#include <cstdint>

#include "core/cft.hpp"
#include "utils/Span.hpp"
#include "utils/StringView.hpp"
#include "utils/parse_utils.hpp"
#include "utils/print.hpp"

namespace cft {

#define CFT_HELP_FLAG      "-h"
#define CFT_HELP_LONG_FLAG "--help"
#define CFT_HELP_HELP      "Print this help message."

#define CFT_INST_FLAG      "-i"
#define CFT_INST_LONG_FLAG "--inst"
#define CFT_INST_HELP      "Instance file path."

#define CFT_PARSER_FLAG      "-p"
#define CFT_PARSER_LONG_FLAG "--parser"
#define CFT_PARSER_HELP                                                            \
    "Available parsers: " CFT_RAIL_PARSER ", " CFT_SCP_PARSER ", " CFT_CVRP_PARSER \
    ", " CFT_MPS_PARSER "."

#define CFT_OUTSOL_FLAG      "-o"
#define CFT_OUTSOL_LONG_FLAG "--out-sol"
#define CFT_OUTSOL_HELP      "File where the solution is written."

#define CFT_INITSOL_FLAG      "-w"
#define CFT_INITSOL_LONG_FLAG "--init-sol"
#define CFT_INITSOL_HELP      "File where the initial solution is read."

#define CFT_SEED_FLAG      "-s"
#define CFT_SEED_LONG_FLAG "--seed"
#define CFT_SEED_HELP      "Random seed."

#define CFT_TLIM_FLAG      "-t"
#define CFT_TLIM_LONG_FLAG "--timelimit"
#define CFT_TLIM_HELP      "Time limit in seconds."

#define CFT_VERBOSE_FLAG      "-v"
#define CFT_VERBOSE_LONG_FLAG "--verbose"
#define CFT_VERBOSE_HELP      "Verbosity level, from 0 to 5."

#define CFT_EPSILON_FLAG      "-e"
#define CFT_EPSILON_LONG_FLAG "--epsilon"
#define CFT_EPSILON_HELP      "Delta to consider two costs different."

#define CFT_GITERS_FLAG      "-g"
#define CFT_GITERS_LONG_FLAG "--heur-iters"
#define CFT_GITERS_HELP      "Number of times the greedy is run."

#define CFT_BETA_FLAG      "-b"
#define CFT_BETA_LONG_FLAG "--beta"
#define CFT_BETA_HELP      "Relative cutoff value to terminate Refinement."

#define CFT_ABSSGEXIT_FLAG      "-a"
#define CFT_ABSSGEXIT_LONG_FLAG "--abs-subg-exit"
#define CFT_ABSSGEXIT_HELP      "Minimum LBs delta to trigger subradient termination."

#define CFT_RELSGEXIT_FLAG      "-r"
#define CFT_RELSGEXIT_LONG_FLAG "--rel-subg-exit"
#define CFT_RELSGEXIT_HELP      "Minimum LBs gap to trigger subradient termination."

namespace local { namespace {
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

inline void print_arg_values(Environment const& env) {
    print<2>(env, " {:20} = {}\n", CFT_INST_FLAG "," CFT_INST_LONG_FLAG, env.inst_path);
    print<2>(env, " {:20} = {}\n", CFT_PARSER_FLAG "," CFT_PARSER_LONG_FLAG, env.parser);
    print<2>(env, " {:20} = {}\n", CFT_OUTSOL_FLAG "," CFT_OUTSOL_LONG_FLAG, env.sol_path);
    print<2>(env, " {:20} = {}\n", CFT_INITSOL_FLAG "," CFT_INITSOL_LONG_FLAG, env.initsol_path);
    print<2>(env, " {:20} = {}\n", CFT_SEED_FLAG "," CFT_SEED_LONG_FLAG, env.seed);
    print<2>(env, " {:20} = {}\n", CFT_TLIM_FLAG "," CFT_TLIM_LONG_FLAG, env.time_limit);
    print<2>(env, " {:20} = {}\n", CFT_VERBOSE_FLAG "," CFT_VERBOSE_LONG_FLAG, env.verbose);
    print<2>(env, " {:20} = {}\n", CFT_EPSILON_FLAG "," CFT_EPSILON_LONG_FLAG, env.epsilon);
    print<2>(env, " {:20} = {}\n", CFT_GITERS_FLAG "," CFT_GITERS_LONG_FLAG, env.heur_iters);
    print<2>(env, " {:20} = {}\n", CFT_BETA_FLAG "," CFT_BETA_LONG_FLAG, env.beta);
    print<2>(env,
             " {:20} = {}\n",
             CFT_ABSSGEXIT_FLAG "," CFT_ABSSGEXIT_LONG_FLAG,
             env.abs_subgrad_exit);
    print<2>(env,
             " {:20} = {}\n\n",
             CFT_RELSGEXIT_FLAG "," CFT_RELSGEXIT_LONG_FLAG,
             env.rel_subgrad_exit);
    std::fflush(stdout);
}

inline void print_cli_help_msg() {
    fmt::print("Commandline arguments available as of compile date: " __DATE__ "\n");
    fmt::print("  {:20} " CFT_HELP_HELP "\n", CFT_HELP_FLAG "," CFT_HELP_LONG_FLAG);
    fmt::print("  {:20} " CFT_INST_HELP "\n", CFT_INST_FLAG "," CFT_INST_LONG_FLAG);
    fmt::print("  {:20} " CFT_PARSER_HELP "\n", CFT_PARSER_FLAG "," CFT_PARSER_LONG_FLAG);
    fmt::print("  {:20} " CFT_OUTSOL_HELP "\n", CFT_OUTSOL_FLAG "," CFT_OUTSOL_LONG_FLAG);
    fmt::print("  {:20} " CFT_INITSOL_HELP "\n", CFT_INITSOL_FLAG "," CFT_INITSOL_LONG_FLAG);
    fmt::print("  {:20} " CFT_SEED_HELP "\n", CFT_SEED_FLAG "," CFT_SEED_LONG_FLAG);
    fmt::print("  {:20} " CFT_TLIM_HELP "\n", CFT_TLIM_FLAG "," CFT_TLIM_LONG_FLAG);
    fmt::print("  {:20} " CFT_VERBOSE_HELP "\n", CFT_VERBOSE_FLAG "," CFT_VERBOSE_LONG_FLAG);
    fmt::print("  {:20} " CFT_EPSILON_HELP "\n", CFT_EPSILON_FLAG "," CFT_EPSILON_LONG_FLAG);
    fmt::print("  {:20} " CFT_GITERS_HELP "\n", CFT_GITERS_FLAG "," CFT_GITERS_LONG_FLAG);
    fmt::print("  {:20} " CFT_BETA_HELP "\n", CFT_BETA_FLAG "," CFT_BETA_LONG_FLAG);
    fmt::print("  {:20} " CFT_ABSSGEXIT_HELP "\n", CFT_ABSSGEXIT_FLAG "," CFT_ABSSGEXIT_LONG_FLAG);
    fmt::print("  {:20} " CFT_RELSGEXIT_HELP "\n", CFT_RELSGEXIT_FLAG "," CFT_RELSGEXIT_LONG_FLAG);
    fmt::print("\n");
    fmt::print("Default values:\n");
    print_arg_values(Environment{});
    fmt::print("\n");
    std::fflush(stdout);
}

inline Environment parse_cli_args(int argc, char const** argv) {
    auto args = cft::make_span(argv, checked_cast<size_t>(argc));
    auto env  = Environment{};

    for (size_t a = 1; a < args.size(); ++a) {
        auto arg = StringView(args[a]);
        if (arg == CFT_HELP_FLAG || arg == CFT_HELP_LONG_FLAG)
            print_cli_help_msg();
        else if (arg == CFT_INST_FLAG || arg == CFT_INST_LONG_FLAG)
            env.inst_path = args[++a];
        else if (arg == CFT_PARSER_FLAG || arg == CFT_PARSER_LONG_FLAG)
            env.parser = args[++a];
        else if (arg == CFT_OUTSOL_FLAG || arg == CFT_OUTSOL_LONG_FLAG)
            env.sol_path = args[++a];
        else if (arg == CFT_INITSOL_FLAG || arg == CFT_INITSOL_LONG_FLAG)
            env.initsol_path = args[++a];
        else if (arg == CFT_SEED_FLAG || arg == CFT_SEED_LONG_FLAG) {
            env.seed = string_to<uint64_t>::parse(args[++a]);
            env.rnd  = {env.seed};
        } else if (arg == CFT_TLIM_FLAG || arg == CFT_TLIM_LONG_FLAG)
            env.time_limit = string_to<double>::parse(args[++a]);
        else if (arg == CFT_VERBOSE_FLAG || arg == CFT_VERBOSE_LONG_FLAG)
            env.verbose = (a + 1 < args.size() && std::isdigit(args[a + 1][0]) != 0
                               ? string_to<uint64_t>::parse(args[++a])
                               : 5);
        else if (arg == CFT_EPSILON_FLAG || arg == CFT_EPSILON_LONG_FLAG)
            env.epsilon = string_to<real_t>::parse(args[++a]);
        else if (arg == CFT_GITERS_FLAG || arg == CFT_GITERS_LONG_FLAG)
            env.heur_iters = string_to<uint64_t>::parse(args[++a]);
        else if (arg == CFT_BETA_FLAG || arg == CFT_BETA_LONG_FLAG)
            env.beta = string_to<real_t>::parse(args[++a]);
        else if (arg == CFT_ABSSGEXIT_FLAG || arg == CFT_ABSSGEXIT_LONG_FLAG)
            env.abs_subgrad_exit = string_to<real_t>::parse(args[++a]);
        else if (arg == CFT_RELSGEXIT_FLAG || arg == CFT_RELSGEXIT_LONG_FLAG)
            env.rel_subgrad_exit = string_to<real_t>::parse(args[++a]);
        else
            fmt::print("Arg '{}' unrecognized, ignored.\n", arg.data());
    }

    if (env.inst_path.empty())
        throw std::runtime_error("Instance file path not provided.");

    if (env.sol_path.empty())
        env.sol_path = local::make_sol_name(env.inst_path);

    return env;
}
}  // namespace cft


#endif /* CFT_SRC_CORE_CLIARGS_HPP */
