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
    fmt::print("Instance summary:\n");
    fmt::print("  nrows:     {}\n", fdata.inst.rows.size());
    fmt::print("  ncols:     {}\n", fdata.inst.cols.size());
    fmt::print("  costs:     {} {} {} {} ...\n",
               fdata.inst.costs[0],
               fdata.inst.costs[1],
               fdata.inst.costs[2],
               fdata.inst.costs[3]);
    fmt::print("  solcosts:  {} {} {} {} ...\n",
               fdata.inst.solcosts[0],
               fdata.inst.solcosts[1],
               fdata.inst.solcosts[2],
               fdata.inst.solcosts[3]);
    if (!fdata.warmstart.empty())
        fmt::print("  warmstart: {} {} {} {} ...\n",
                   fdata.warmstart[0],
                   fdata.warmstart[1],
                   fdata.warmstart[2],
                   fdata.warmstart[3]);

    // print first 10 columns
    for (size_t i = 0; i < 4; ++i)
        fmt::print("  col[{}]: {}\n", i, fmt::join(fdata.inst.cols[i], ", "));
}

int main(int argc, char const** argv) {

    auto args       = cft::make_span(argv, argc);
    auto inst       = cft::parse_rail_instance(args[1]);
    auto greedy     = cft::Greedy();
    auto fixing     = cft::make_identity_fixing_data(inst.cols.size(), inst.rows.size());
    auto col_fixing = cft::ColFixing();
    auto rnd        = cft::prng_t{0};
    auto best_sol   = cft::Solution();

    while (!inst.rows.empty()) {
        constexpr cft::ridx_t min_row_coverage = 5;

        auto core      = cft::build_tentative_core_instance(inst, min_row_coverage);
        auto lagr_mult = cft::compute_greedy_multipliers(core.inst);
        auto sol       = cft::Solution();
        greedy(core.inst, lagr_mult, sol);

        cft::real_t step_size = 0.1;
        auto        cutoff    = std::min(sol.cost, best_sol.cost - fixing.fixed_cost);
        auto        opt_lb    = cft::optimize(inst, core, cutoff, sol.cost, step_size, lagr_mult);
        auto        exp_lb    = cft::explore(core.inst, greedy, cutoff, step_size, sol, lagr_mult);

        auto best_lb = std::max(opt_lb, exp_lb);
        if (best_lb >= cutoff - CFT_EPSILON) {
            fmt::print("Early exit: best_lb: {} >= cutoff: {}\n", best_lb, cutoff - CFT_EPSILON);
            break;
        }

        fmt::print("Best lower bound: {}, best solution cost: {}\n",
                   best_lb + fixing.fixed_cost,
                   sol.cost + fixing.fixed_cost);

        if (sol.cost + fixing.fixed_cost < best_sol.cost) {
            best_sol.cost = sol.cost + fixing.fixed_cost;
            best_sol.idxs = fixing.fixed_cols;
            for (cft::cidx_t j : sol.idxs)
                best_sol.idxs.push_back(fixing.curr2orig_col_map[j]);
            IF_DEBUG(check_solution(core.inst, sol));
            // TODO(any): cannot check best_sol atm since we loose the original instance
        }

        // TODO(cava): Col fixing for inst considering core-inst?
        col_fixing(inst, core.inst, fixing, lagr_mult, sol, greedy);
        cft::perturb_lagr_multipliers(lagr_mult, rnd);
        fmt::print("Fixing: rows left: {}, fixed cost: {}\n", inst.rows.size(), fixing.fixed_cost);
        cft::perturb_lagr_multipliers(lagr_mult, rnd);
    }

    fmt::print("\nBest solution cost: {}\n", best_sol.cost);

    return EXIT_SUCCESS;
}
