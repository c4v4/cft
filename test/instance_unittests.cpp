#include <catch2/catch.hpp>
#include <cstring>

#include "core/coverage.hpp"
#include "instance/Instance.hpp"
#include "instance/parsing.hpp"

namespace cft {
// TODO(cava): make tests for these functions

// void Instance::invariants_check();

// void fix_columns(std::vector<cidx_t> const& cols_to_fix, IdxMaps& idx_maps);

// inline void complete_init(Instance& partial_inst, ridx_t nrows);

TEST_CASE("test_build_tentative_core_instance") {

    int min_row_coverage = 2;

    auto  file_data = parse_scp_instance("../instances/scp/scp41.txt");
    auto& inst      = file_data.inst;
    auto  core_inst = build_tentative_core_instance(inst, min_row_coverage);

    ridx_t nrows = inst.rows.size();

    auto cov = CoverCounters<>(nrows);
    for (cidx_t j = 0; j < core_inst.cols.size(); ++j)
        cov.cover(core_inst.cols[j]);

    for (ridx_t i = 0; i < nrows; ++i)
        REQUIRE(cov[i] >= min_row_coverage);
}

}  // namespace cft