#include <catch2/catch.hpp>

#include "algorithms/ThreePhase.hpp"
#include "core/coverage.hpp"
#include "core/sort.hpp"
#include "instance/Instance.hpp"
#include "instance/parsing.hpp"

namespace cft {

TEST_CASE("test_build_tentative_core_instance") {

    int min_row_coverage = 2;

    auto   inst   = parse_scp_instance("../instances/scp/scp41.txt");
    auto   sorter = Sorter();
    auto   core   = build_tentative_core_instance(inst, sorter, min_row_coverage);
    ridx_t nrows  = inst.rows.size();

    auto cov = CoverCounters<>(nrows);
    for (cidx_t j = 0; j < core.inst.cols.size(); ++j)
        cov.cover(core.inst.cols[j]);

    for (ridx_t i = 0; i < nrows; ++i)
        REQUIRE(cov[i] >= min_row_coverage);
}

}  // namespace cft