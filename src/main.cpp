#include <fmt/core.h>
#include <fmt/ranges.h>

#include "core/cft.hpp"
#include "core/coverage.hpp"
#include "core/limits.hpp"
#include "fixing/ColFixing.hpp"
#include "fixing/fix_columns.hpp"
#include "greedy/Greedy.hpp"
#include "instance/Instance.hpp"
#include "instance/parsing.hpp"
#include "subgradient/subgradient.hpp"

void print_inst_summary(cft::FileData const& fdata) {
    fmt::print("3PHS > Instance summary:\n");
    fmt::print("3PHS >   nrows:     {}\n", fdata.inst.rows.size());
    fmt::print("3PHS >   ncols:     {}\n", fdata.inst.cols.size());
    fmt::print("3PHS >   costs:     {} {} {} {} ...\n",
               fdata.inst.costs[0],
               fdata.inst.costs[1],
               fdata.inst.costs[2],
               fdata.inst.costs[3]);
    fmt::print("3PHS >   solcosts:  {} {} {} {} ...\n",
               fdata.inst.solcosts[0],
               fdata.inst.solcosts[1],
               fdata.inst.solcosts[2],
               fdata.inst.solcosts[3]);
    if (!fdata.warmstart.empty())
        fmt::print("3PHS >   warmstart: {} {} {} {} ...\n",
                   fdata.warmstart[0],
                   fdata.warmstart[1],
                   fdata.warmstart[2],
                   fdata.warmstart[3]);

    // print first 10 columns
    for (size_t i = 0; i < 4; ++i)
        fmt::print("3PHS >   col[{}]: {}\n", i, fmt::join(fdata.inst.cols[i], ", "));
}

int main(int argc, char const** argv) {

    auto args       = cft::make_span(argv, argc);
    auto inst       = cft::parse_rail_instance(args[1]);
    auto greedy     = cft::Greedy();
    auto fixing     = cft::make_identity_fixing_data(inst.cols.size(), inst.rows.size());
    auto col_fixing = cft::ColFixing();
    auto rnd        = cft::prng_t{0};
    auto best_sol   = cft::Solution();
    auto sorter     = cft::Sorter();

    while (!inst.rows.empty()) {
        constexpr cft::ridx_t min_row_coverage = 5;

        auto core      = cft::build_tentative_core_instance(inst, sorter, min_row_coverage);
        auto lagr_mult = cft::compute_greedy_multipliers(core.inst);
        auto sol       = cft::Solution();
        greedy(core.inst, lagr_mult, sol);

        cft::real_t step_size = 0.1;
        auto        cutoff    = std::min(sol.cost, best_sol.cost - fixing.fixed_cost);

        auto opt_lb = cft::optimize(inst, core, sorter, cutoff, sol.cost, step_size, lagr_mult);
        if (opt_lb >= cutoff - CFT_EPSILON)
            break;

        auto exp_lb = cft::explore(core.inst, greedy, cutoff, step_size, sol, lagr_mult);
        if (exp_lb >= cutoff - CFT_EPSILON)
            break;

        fmt::print("3PHS > Best lower bound: {}, best solution cost: {}\n",
                   cft::max(opt_lb, exp_lb) + fixing.fixed_cost,
                   sol.cost + fixing.fixed_cost);

        if (sol.cost + fixing.fixed_cost < best_sol.cost) {
            best_sol.cost = sol.cost + fixing.fixed_cost;
            best_sol.idxs = fixing.fixed_cols;
            for (cft::cidx_t j : sol.idxs)
                best_sol.idxs.push_back(fixing.curr2orig_col_map[j]);
            IF_DEBUG(check_solution(core.inst, sol));
            // TODO(any): cannot check best_sol atm since we loose the original instance
        }

        col_fixing(inst, core, fixing, lagr_mult, greedy);
        fmt::print("3PHS > Fixing: rows left: {}, fixed cost: {}\n",
                   inst.rows.size(),
                   fixing.fixed_cost);
        cft::perturb_lagr_multipliers(lagr_mult, rnd);
    }

    fmt::print("\n3PHS > Best solution cost: {}\n", best_sol.cost);

    return EXIT_SUCCESS;
}
