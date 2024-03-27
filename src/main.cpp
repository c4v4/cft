#include <fmt/core.h>
#include <fmt/ranges.h>

#include "algorithms/Refinement.hpp"
#include "core/cft.hpp"
#include "instance/Instance.hpp"
#include "instance/parsing.hpp"

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

    auto args = cft::make_span(argv, argc);
    auto inst = cft::parse_rail_instance(args[1]);
    auto rnd  = cft::prng_t{0};

    auto sol = cft::run(inst, rnd);
    fmt::print("REFN > Best solution cost: {:.2f}\n", sol.cost);

    return EXIT_SUCCESS;
}
