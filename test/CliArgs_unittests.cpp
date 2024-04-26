// SPDX-FileCopyrightText: 2024 Francesco Cavaliere <francescocava95@gmail.com>
// SPDX-License-Identifier: MIT

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#define DOCTEST_CONFIG_SUPER_FAST_ASSERTS

#include <doctest/doctest.h>

#include <stdexcept>

#include "core/CliArgs.hpp"
#include "core/cft.hpp"

namespace cft {
TEST_CASE("parse_cli_args parses command line arguments correctly") {
    char const* argv[] = {"program_name", "-i",       "input.txt", "-p", "rail", "-o",
                          "output.sol",   "-s",       "12345",     "-t", "10.0", "-v",
                          " 5",           "-e",       "1E-3",      "-g", "100",  "-b",
                          "0.5",          "-a",       "1e-2",      "-r", "1E-1", "-h",
                          "-w",           "test.sol", "-U"};

    int  argc = sizeof(argv) / sizeof(argv[0]);
    auto env  = parse_cli_args(argc, argv);

    CHECK(env.inst_path == "input.txt");
    CHECK(env.parser == "rail");
    CHECK(env.sol_path == "output.sol");
    CHECK(env.initsol_path == "test.sol");
    CHECK(env.seed == 12345);
    CHECK(env.time_limit == 10.0);
    CHECK(env.verbose == 5);
    CHECK(env.epsilon == 0.001_F);
    CHECK(env.heur_iters == 100);
    CHECK(env.beta == 0.5_F);
    CHECK(env.abs_subgrad_exit == 0.01_F);
    CHECK(env.rel_subgrad_exit == 0.1_F);

    CHECK_NOTHROW(print_cli_help_msg());
    CHECK_NOTHROW(print_arg_values(env));
}

TEST_CASE("parse_cli_args no args") {
    char const* argv[] = {"program_name"};
    int         argc   = sizeof(argv) / sizeof(argv[0]);
    CHECK_THROWS_AS(parse_cli_args(argc, argv), std::runtime_error);
}

TEST_CASE("parse_cli_args instance only") {
    char const* argv[] = {"program_name", "-i", "input.txt"};
    int         argc   = sizeof(argv) / sizeof(argv[0]);
    auto        env    = parse_cli_args(argc, argv);

    CHECK(env.inst_path == "input.txt");
    CHECK(env.parser == "RAIL");
    CHECK(env.sol_path == "input.sol");
    CHECK(env.initsol_path.empty());
    CHECK(env.seed == 0);
    CHECK(env.time_limit == limits<double>::inf());
    CHECK(env.verbose == 4);
    CHECK(env.epsilon == 0.999_F);
    CHECK(env.heur_iters == 250);
    CHECK(env.alpha == 1.1_F);
    CHECK(env.beta == 1.0_F);
    CHECK(env.abs_subgrad_exit == 1.0_F);
    CHECK(env.rel_subgrad_exit == 0.001_F);
}

TEST_CASE("parse_cli_args parses command line arguments correctly (long)") {
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
                          "--unrecognized",
                          "--rel-subg-exit",
                          "0.1",
                          "--help",
                          "--init-sol",
                          "test.sol"};

    int  argc = sizeof(argv) / sizeof(argv[0]);
    auto env  = parse_cli_args(argc, argv);

    CHECK(env.inst_path == "input.txt");
    CHECK(env.parser == CFT_RAIL_PARSER);
    CHECK(env.sol_path == "output.sol");
    CHECK(env.initsol_path == "test.sol");
    CHECK(env.seed == 12345);
    CHECK(env.time_limit == 10.0);
    CHECK(env.verbose == 2);
    CHECK(env.epsilon == 0.001_F);
    CHECK(env.heur_iters == 100);
    CHECK(env.beta == 0.5_F);
    CHECK(env.abs_subgrad_exit == 0.01_F);
    CHECK(env.rel_subgrad_exit == 0.1_F);

    CHECK_NOTHROW(print_cli_help_msg());
    CHECK_NOTHROW(print_arg_values(env));
}

TEST_CASE("make_sol_name returns correct solution name with path and extension") {
    std::string inst_path         = "/home/user/repos/cft/instances/problem.txt";
    std::string expected_sol_name = "problem.sol";
    CHECK(local::make_sol_name(inst_path) == expected_sol_name);
}

TEST_CASE("make_sol_name returns correct solution name with path and multiple dots in "
          "filename") {
    std::string inst_path         = "../../cft/instances/problem.test.txt";
    std::string expected_sol_name = "problem.test.sol";
    CHECK(local::make_sol_name(inst_path) == expected_sol_name);
}

TEST_CASE("make_sol_name returns correct solution name without path") {
    std::string inst_path         = "problem.txt";
    std::string expected_sol_name = "problem.sol";
    CHECK(local::make_sol_name(inst_path) == expected_sol_name);
}

TEST_CASE("make_sol_name returns correct solution name without extension") {
    std::string inst_path         = "./cft/instances/problem";
    std::string expected_sol_name = "problem.sol";
    CHECK(local::make_sol_name(inst_path) == expected_sol_name);
}

}  // namespace cft
