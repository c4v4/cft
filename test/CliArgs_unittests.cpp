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

#include <catch2/catch.hpp>
#include <stdexcept>

#include "core/CliArgs.hpp"
#include "core/cft.hpp"

namespace cft {
TEST_CASE("parse_cli_args parses command line arguments correctly", "[parse_cli_args]") {
    char const* argv[] = {"program_name", "-i",       "input.txt", "-p", "rail", "-o",
                          "output.sol",   "-s",       "12345",     "-t", "10.0", "-v",
                          " 5",           "-e",       "1E-3",      "-g", "100",  "-b",
                          "0.5",          "-a",       "1e-2",      "-r", "1E-1", "-h",
                          "-w",           "test.sol", "-U"};

    int argc = sizeof(argv) / sizeof(argv[0]);

    SECTION("parse_cli_args parses command line arguments correctly") {

        auto env = parse_cli_args(argc, argv);

        REQUIRE(env.inst_path == "input.txt");
        REQUIRE(env.parser == "rail");
        REQUIRE(env.sol_path == "output.sol");
        REQUIRE(env.initsol_path == "test.sol");
        REQUIRE(env.seed == 12345);
        REQUIRE(env.time_limit == 10.0);
        REQUIRE(env.verbose == 5);
        REQUIRE(env.epsilon == 0.001_F);
        REQUIRE(env.heur_iters == 100);
        REQUIRE(env.beta == 0.5_F);
        REQUIRE(env.abs_subgrad_exit == 0.01_F);
        REQUIRE(env.rel_subgrad_exit == 0.1_F);

        REQUIRE_NOTHROW(print_cli_help_msg());
        REQUIRE_NOTHROW(print_arg_values(env));
    }
}

TEST_CASE("parse_cli_args no args", "[parse_cli_args]") {
    char const* argv[] = {"program_name"};

    int argc = sizeof(argv) / sizeof(argv[0]);

    SECTION("parse_cli_args parses no arguments correctly") {
        REQUIRE_THROWS_AS(parse_cli_args(argc, argv), std::runtime_error);
    }
}

TEST_CASE("parse_cli_args instance only", "[parse_cli_args]") {
    char const* argv[] = {"program_name", "-i", "input.txt"};

    int argc = sizeof(argv) / sizeof(argv[0]);

    SECTION("parse_cli_args parses instance only correclty") {
        auto env = parse_cli_args(argc, argv);

        REQUIRE(env.inst_path == "input.txt");
        REQUIRE(env.parser == "RAIL");
        REQUIRE(env.sol_path == "input.sol");
        REQUIRE(env.initsol_path.empty());
        REQUIRE(env.seed == 0);
        REQUIRE(env.time_limit == limits<double>::inf());
        REQUIRE(env.verbose == 4);
        REQUIRE(env.epsilon == 0.999999_F);
        REQUIRE(env.heur_iters == 250);
        REQUIRE(env.alpha == 1.1_F);
        REQUIRE(env.beta == 1.0_F);
        REQUIRE(env.abs_subgrad_exit == 1.0_F);
        REQUIRE(env.rel_subgrad_exit == 0.001_F);
    }
}

TEST_CASE("parse_cli_args parses command line arguments correctly (long)", "[parse_cli_args]") {
    char const* argv[] = {"program_name",
                          "--inst",
                          "input.txt",
                          "--parser",
                          CFT_RAIL_PARSER,
                          "--out-sol",
                          "output.sol",
                          "--seed",
                          "12345",
                          "--timelimit",
                          "1e1",
                          "--verbose",
                          "2",
                          "--epsilon",
                          "1E-3",
                          "--heur-iters",
                          "100",
                          "--beta",
                          "0.5",
                          "--abs-subg-exit",
                          "0.01",
                          "--rel-subg-exit",
                          "0.1",
                          "--help",
                          "--init-sol",
                          "test.sol",
                          "--unrecognized"};

    int argc = sizeof(argv) / sizeof(argv[0]);

    SECTION("parse_cli_args parses command line arguments correctly") {
        auto env = parse_cli_args(argc, argv);

        REQUIRE(env.inst_path == "input.txt");
        REQUIRE(env.parser == CFT_RAIL_PARSER);
        REQUIRE(env.sol_path == "output.sol");
        REQUIRE(env.initsol_path == "test.sol");
        REQUIRE(env.seed == 12345);
        REQUIRE(env.time_limit == 10.0);
        REQUIRE(env.verbose == 2);
        REQUIRE(env.epsilon == 0.001_F);
        REQUIRE(env.heur_iters == 100);
        REQUIRE(env.beta == 0.5_F);
        REQUIRE(env.abs_subgrad_exit == 0.01_F);
        REQUIRE(env.rel_subgrad_exit == 0.1_F);

        REQUIRE_NOTHROW(print_cli_help_msg());
        REQUIRE_NOTHROW(print_arg_values(env));
    }
}

TEST_CASE("make_sol_name returns correct solution name", "[make_sol_name]") {
    SECTION("With path and extension") {
        std::string inst_path         = "/home/user/repos/cft/instances/problem.txt";
        std::string expected_sol_name = "problem.sol";
        REQUIRE(local::make_sol_name(inst_path) == expected_sol_name);
    }

    SECTION("With path and multiple dots in filename") {
        std::string inst_path         = "../../cft/instances/problem.test.txt";
        std::string expected_sol_name = "problem.test.sol";
        REQUIRE(local::make_sol_name(inst_path) == expected_sol_name);
    }

    SECTION("Without path") {
        std::string inst_path         = "problem.txt";
        std::string expected_sol_name = "problem.sol";
        REQUIRE(local::make_sol_name(inst_path) == expected_sol_name);
    }

    SECTION("Without extension") {
        std::string inst_path         = "./cft/instances/problem";
        std::string expected_sol_name = "problem.sol";
        REQUIRE(local::make_sol_name(inst_path) == expected_sol_name);
    }
}

}  // namespace cft
