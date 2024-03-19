#include <catch2/catch.hpp>
#include <cstring>

#include "core/coverage.hpp"
#include "instance/Instance.hpp"

namespace cft {
// TODO(cava): make tests for these functions

// void Instance::invariants_check();

// void fix_columns(std::vector<cidx_t> const& cols_to_fix, IdxMaps& idx_maps);

// inline void complete_init(Instance& partial_inst, ridx_t nrows);

// Instance make_instance(InstanceData&& inst_data);

// Instance make_instance(InstanceData const& inst_data);

TEST_CASE("test_make_tentative_core_instance") {

    int min_row_coverage = 2;

    auto inst      = make_instance(parse_scp_instance("../instances/scp/scp41.txt"));
    auto core_inst = make_tentative_core_instance(inst, min_row_coverage);

    ridx_t nrows = inst.rows.size();

    auto cov = make_cover_counters(nrows);
    for (cidx_t j = 0; j < core_inst.cols.size(); ++j)
        cov.cover(core_inst.cols[j]);

    for (ridx_t i = 0; i < nrows; ++i)
        REQUIRE(cov[i] >= min_row_coverage);
}

}  // namespace cft