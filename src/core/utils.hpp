// SPDX-FileCopyrightText: 2024 Francesco Cavaliere <francescocava95@gmail.com>
// SPDX-License-Identifier: MIT

#ifndef CFT_SRC_CORE_UTILS_HPP
#define CFT_SRC_CORE_UTILS_HPP

#include "core/Instance.hpp"
#include "core/cft.hpp"

#ifndef NDEBUG
#include "utils/CoverCounters.hpp"
#endif

namespace cft {
#ifndef NDEBUG
// Check solution cost and feasibility.
inline void check_inst_solution(Instance const& inst, Solution const& sol) {
    ridx_t const nrows = rsize(inst.rows);

    // check coverage
    ridx_t covered_rows = 0_R;
    auto   row_coverage = CoverCounters(nrows);
    for (auto j : sol.idxs)
        covered_rows += as_ridx(row_coverage.cover(inst.cols[j]));
    assert(covered_rows == nrows);

    // check cost
    real_t total_cost = 0.0_F;
    for (cidx_t j : sol.idxs)
        total_cost += inst.costs[j];
    assert(abs(total_cost - sol.cost) < 1e-6_F);
}
#endif

// Compute reduced costs for all the columns of an instance given a set of multipliers.
// Hook can be used to perform additional operations on each reduced cost.
template <typename Hook = NoOp>
void compute_reduced_costs(Instance const&            inst,           // in
                           std::vector<real_t> const& multipliers,    // in
                           std::vector<real_t>&       reduced_costs,  // out
                           Hook                       hook = NoOp{}   // in
) {
    assert(rsize(multipliers) == rsize(inst.rows) && "Invalid multipliers size");
    cidx_t const ncols = csize(inst.cols);

    reduced_costs.resize(ncols);
    for (cidx_t j = 0_C; j < ncols; ++j) {

        reduced_costs[j] = inst.costs[j];
        for (ridx_t i : inst.cols[j])
            reduced_costs[j] -= multipliers[i];
        hook(reduced_costs[j], j);
    }
}

}  // namespace cft
#endif /* CFT_SRC_CORE_UTILS_HPP */
