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

#include "algorithms/Refinement.hpp"
#include "core/CliArgs.hpp"
#include "core/cft.hpp"
#include "core/parsing.hpp"
#include "utils/print.hpp"

int main(int argc, char const** argv) {

    try {
        auto env = cft::parse_cli_args(argc, argv);

        cft::print<1>(env, "CFT implementation by Luca Accorsi and Francesco Cavaliere.\n");
        cft::print<1>(env, "Compiled on " __DATE__ " at " __TIME__ ".\n\n");
        cft::print<2>(env, "Running with parameters set to:\n");
        cft::print_arg_values(env);

        auto fdata = cft::parse_inst_and_initsol(env);
        auto sol   = cft::run(env, fdata.inst, fdata.init_sol);
        cft::write_solution(env.sol_path, sol);
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
