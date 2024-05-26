// SPDX-FileCopyrightText: 2024 Francesco Cavaliere <francescocava95@gmail.com>
// SPDX-License-Identifier: MIT

#include "algorithms/Refinement.hpp"
#include "core/CliArgs.hpp"
#include "core/cft.hpp"
#include "core/parsing.hpp"
#include "utils/print.hpp"

int main(int argc, char const** argv) {

    try {
        auto env = cft::parse_cli_args(argc, argv);

        cft::print<1>(env, "CFT implementation by Luca Accorsi and Francesco Cavaliere.\n");
        cft::print<2>(env, "Compiled on " __DATE__ " at " __TIME__ ".\n\n");
        cft::print<3>(env, "Running with parameters set to:\n");
        cft::print_arg_values(env);

        auto fdata = cft::parse_inst_and_initsol(env);
        auto sol   = cft::run(env, fdata.inst, fdata.init_sol);
        cft::write_solution(env.sol_path, sol);
        cft::print<1>(env,
                      "CFT> Best solution {:.2f} time {:.2f}s\n",
                      sol.cost,
                      env.timer.elapsed<cft::sec>());

    } catch (std::exception const& e) {
        fmt::print(stderr, "\nCFT> ERROR: {}\n", e.what());
        std::fflush(stdout);
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
