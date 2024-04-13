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

#include "core/CliArgs.hpp"

TEST_CASE("parse_cli_args parses command line arguments correctly", "[parse_cli_args]") {
    char const* argv[] = {"program_name", "-i", "input.txt", "-p", "rail", "-o", "output.sol", "-s",
                          "12345",        "-t", "10.0",      "-v", "2",    "-e", "0.001",      "-g",
                          "100",          "-b", "0.5",       "-a", "1e-2", "-r", "1E-1"};

    int argc = sizeof(argv) / sizeof(argv[0]);

    SECTION("parse_cli_args parses command line arguments correctly") {
        auto cli_args = cft::parse_cli_args(argc, argv);

        REQUIRE(cli_args.inst_path == "input.txt");
        REQUIRE(cli_args.parser == "rail");
        REQUIRE(cli_args.sol_path == "output.sol");
        REQUIRE(cli_args.seed == 12345);
        REQUIRE(cli_args.time_limit == 10.0);
        REQUIRE(cli_args.verbose == 2);
        REQUIRE(cli_args.epsilon == 0.001F);
        REQUIRE(cli_args.heur_iters == 100);
        REQUIRE(cli_args.beta == 0.5F);
        REQUIRE(cli_args.abs_subgrad_exit == 0.01F);
        REQUIRE(cli_args.rel_subgrad_exit == 0.1F);
    }
}

TEST_CASE("parse_cli_args throw missing inst path", "[parse_cli_args]") {
    char const* argv[] = {"program_name", "-p",   "rail", "-o", "output.sol", "-s",    "12345",
                          "-t",           "10.0", "-v",   "2",  "-e",         "0.001", "-g",
                          "100",          "-b",   "0.5",  "-a", "1e-2",       "-r",    "1E-1"};

    int argc = sizeof(argv) / sizeof(argv[0]);

    SECTION("parse_cli_args throws exception if instance file path is not provided") {
        REQUIRE_THROWS_WITH(cft::parse_cli_args(argc, argv), "Instance file path not provided.");
    }
}

TEST_CASE("parse_cli_args parses command line arguments correctly (long)", "[parse_cli_args]") {
    char const* argv[] = {"program_name",
                          "--inst",
                          "input.txt",
                          "--parser",
                          "rail",
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
                          "0.1"};

    int argc = sizeof(argv) / sizeof(argv[0]);

    SECTION("parse_cli_args parses command line arguments correctly") {
        auto cli_args = cft::parse_cli_args(argc, argv);

        REQUIRE(cli_args.inst_path == "input.txt");
        REQUIRE(cli_args.parser == "rail");
        REQUIRE(cli_args.sol_path == "output.sol");
        REQUIRE(cli_args.seed == 12345);
        REQUIRE(cli_args.time_limit == 10.0);
        REQUIRE(cli_args.verbose == 2);
        REQUIRE(cli_args.epsilon == 0.001F);
        REQUIRE(cli_args.heur_iters == 100);
        REQUIRE(cli_args.beta == 0.5F);
        REQUIRE(cli_args.abs_subgrad_exit == 0.01F);
        REQUIRE(cli_args.rel_subgrad_exit == 0.1F);
    }
}
