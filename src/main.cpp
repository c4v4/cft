#include <fmt/core.h>

#include "CliArgs.hpp"
#include "Instance.hpp"
#include "Refinement.hpp"
#include "cft.hpp"
#include "parsing.hpp"

InstanceData parse_instance_data(CliArgs const& cli) {
    if (cli.parser_type() == CliArgs::ParserType::CVRP)
        return parse_cvrp_instance(cli.path());
    if (cli.parser_type() == CliArgs::ParserType::RAILS)
        return parse_rail_instance(cli.path());
    if (cli.parser_type() == CliArgs::ParserType::SCP)
        return parse_scp_instance(cli.path());
    // Not reachable.
    return {};
}

int main(int argc, char** argv) {

    std::optional<CliArgs> cli = CliArgs::parse(argc, argv);
    if (!cli.has_value()) {
        fmt::print("An error occurred while parsing command line arguments.\n");
        return EXIT_FAILURE;
    }

    auto const data = parse_instance_data(*cli);
    auto       rnd  = cft::prng_t{cli->seed()};

    auto instance = Instance{data.nrows};
    instance.add_columns(data.costs, data.solcosts, data.matbeg, data.matval);

    auto cft      = Refinement{instance, rnd};
    auto solution = cft(data.warmstart);

    real_t sol_cost = 0.0;
    for (auto j : solution)
        sol_cost += instance.get_col(j).get_cost();
    fmt::print("Solution (cost {}):\n{}\n", sol_cost, fmt::join(solution, ", "));

    CountSet coverage;
    coverage.reset_covered(instance.get_cols(), solution, instance.get_nrows());
    fmt::print("Row coverage:\n{}\n", fmt::join(coverage, ", "));


    return 0;
}
