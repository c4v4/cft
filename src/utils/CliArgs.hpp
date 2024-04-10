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


#include "utils/Span.hpp"
#include "utils/StringView.hpp"
#include "utils/parse_utils.hpp"

namespace cft {

// AVAILABLE PARSERS
#define CFT_RAIL_PARSER    "RAIL"
#define CFT_SCP_PARSER     "SCP"
#define CFT_CVRP_PARSER    "CVRP"
#define CFT_MPS_PARSER     "MPS"
#define CFT_DEFAULT_PARSER CFT_RAIL_PARSER

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
    ", " CFT_MPS_PARSER ". Default: " CFT_DEFAULT_PARSER "."

#define CFT_SEED_FLAG      "-s"
#define CFT_SEED_LONG_FLAG "--seed"
#define CFT_SEED_HELP      "Random seed."

#define CFT_TLIM_FLAG      "-t"
#define CFT_TLIM_LONG_FLAG "--timelimit"
#define CFT_TLIM_HELP      "Time limit in seconds."

struct CliArgs {
    std::string inst_path  = {};
    std::string parser     = CFT_DEFAULT_PARSER;
    uint64_t    seed       = 0;
    double      time_limit = limits<double>::inf();
};

static inline void print_cli_help_msg() {
    fmt::print("Commandline arguments available as of compile date: " __DATE__ "\n");
    fmt::print("  {:16} " CFT_HELP_HELP "\n", CFT_HELP_FLAG "," CFT_HELP_LONG_FLAG);
    fmt::print("  {:16} " CFT_INST_HELP "\n", CFT_INST_FLAG "," CFT_INST_LONG_FLAG);
    fmt::print("  {:16} " CFT_PARSER_HELP "\n", CFT_PARSER_FLAG "," CFT_PARSER_LONG_FLAG);
    fmt::print("  {:16} " CFT_SEED_HELP "\n", CFT_SEED_FLAG "," CFT_SEED_LONG_FLAG);
    fmt::print("  {:16} " CFT_TLIM_HELP "\n", CFT_TLIM_FLAG "," CFT_TLIM_LONG_FLAG);
    fmt::print("\n");
    std::fflush(stdout);
}

static inline void print_arg_values(CliArgs const& cli_args) {
    fmt::print("Commandline arguments available as of compile date: " __DATE__ "\n");
    fmt::print(" {:16} = {}\n", CFT_INST_FLAG "," CFT_INST_LONG_FLAG, cli_args.inst_path);
    fmt::print(" {:16} = {}\n", CFT_PARSER_FLAG "," CFT_PARSER_LONG_FLAG, cli_args.parser);
    fmt::print(" {:16} = {}\n", CFT_SEED_FLAG "," CFT_SEED_LONG_FLAG, cli_args.seed);
    fmt::print(" {:16} = {}\n", CFT_TLIM_FLAG "," CFT_TLIM_LONG_FLAG, cli_args.time_limit);
    fmt::print("\n");
    std::fflush(stdout);
}

static inline CliArgs parse_cli_args(int argc, char const** argv) {
    auto args     = cft::make_span(argv, argc);
    auto cli_args = CliArgs{};

    for (size_t a = 1; a < args.size(); ++a) {
        auto arg = StringView(args[a]);
        if (arg == CFT_HELP_FLAG || arg == CFT_HELP_LONG_FLAG)
            print_cli_help_msg();
        else if (arg == CFT_INST_FLAG || arg == CFT_INST_LONG_FLAG)
            cli_args.inst_path = args[++a];
        else if (arg == CFT_PARSER_FLAG || arg == CFT_PARSER_LONG_FLAG)
            cli_args.parser = args[++a];
        else if (arg == CFT_SEED_FLAG || arg == CFT_SEED_LONG_FLAG)
            cli_args.seed = string_to<uint64_t>::parse(args[++a]);
        else if (arg == CFT_TLIM_FLAG || arg == CFT_TLIM_LONG_FLAG)
            cli_args.time_limit = string_to<double>::parse(args[++a]);
        else
            fmt::print("Args '{}' unrecognized, ignored.\n", arg);
    }

    if (cli_args.inst_path.empty()) {
        print_cli_help_msg();
        throw std::runtime_error("Instance file path not provided.");
    }

    return cli_args;
}
}  // namespace cft


#endif /* CFT_SRC_CORE_CLIARGS_HPP */
