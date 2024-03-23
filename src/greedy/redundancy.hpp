// Copyright (c) 2024 Francesco Cavaliere
// This program is free software: you can redistribute it and/or modify it under the terms of the
// GNU General Public License as published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version. This program is distributed in the hope that it
// will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details. You should
// have received a copy of the GNU General Public License along with this program. If not, see
// <https://www.gnu.org/licenses/>.

#ifndef CFT_SRC_GREEDY_REDUNDANCY_HPP
#define CFT_SRC_GREEDY_REDUNDANCY_HPP

#include "core/cft.hpp"
#include "core/coverage.hpp"
#include "core/limits.hpp"
#include "fmt/base.h"
#include "instance/Instance.hpp"


#define CFT_ENUM_VARS 128

namespace cft {

// Data structure to store the redundancy set and related information
struct RedundancyData {
    // TODO(cava): shouldn't change much, but we could use a SparseBinMat to store columns locally

    std::vector<CidxAndCost> redund_set;      // redundant columns + their cost
    CoverCounters<>          total_cover;     // row-cov if all the remaining columns are selected
    CoverCounters<>          partial_cover;   // row-cov if we selected the current column
    std::vector<cidx_t>      cols_to_remove;  // list of columns to remove
    real_t                   best_cost         = limits<real_t>::max();  // current best upper bound
    real_t                   partial_cost      = 0.0;                    // current solution cost
    cidx_t                   partial_cov_count = 0;                      // number of covered rows
};

#ifndef NDEBUG
inline void check_redundancy_data(Instance const&            inst,
                                  std::vector<cidx_t> const& sol,
                                  RedundancyData const&      red_set) {

    auto   total_check    = CoverCounters<>(inst.rows.size());
    auto   part_check     = CoverCounters<>(inst.rows.size());
    size_t part_cov_count = 0;
    for (cidx_t j : sol) {
        cidx_t part_covered = part_check.cover(inst.cols[j]);
        cidx_t tot_covered  = total_check.cover(inst.cols[j]);
        assert(tot_covered == part_covered);
        part_cov_count += part_covered;
    }
    for (cidx_t j : red_set.cols_to_remove) {
        part_cov_count -= part_check.uncover(inst.cols[j]);
        total_check.uncover(inst.cols[j]);
    }
    for (CidxAndCost x : red_set.redund_set)
        part_cov_count -= part_check.uncover(inst.cols[x.col]);

    assert(part_cov_count == red_set.partial_cov_count);
    for (ridx_t i = 0; i < inst.rows.size(); ++i) {
        assert(red_set.total_cover[i] == total_check[i]);
        assert(red_set.partial_cover[i] == part_check[i]);
    }
}
#endif

namespace {
    // When the threshold is below a certain value, we can enumerate all possible
    // non-redundant combinations. As the threshold is known at compile-time, the enumeration has a
    // fixed maximum depth. Enumerator utilizes partial specialization (since `if constexpr` is a
    // C++17 feature) to limit the number of enumeration steps and (technically) remove the
    // recursion.
    template <size_t Cur>
    struct Enumerator;

    template <>
    struct Enumerator<CFT_ENUM_VARS> {
        static void invoke(Instance const& /*inst*/,
                           RedundancyData& red_data,
                           bool (&vars)[CFT_ENUM_VARS],
                           bool (&sol)[CFT_ENUM_VARS]) {

            fmt::print("{}>", CFT_ENUM_VARS);

            if (red_data.partial_cost < red_data.best_cost) {
                red_data.best_cost = red_data.partial_cost;
                for (cidx_t s = 0; s < CFT_ENUM_VARS; ++s)
                    sol[s] = vars[s];
            }

#ifndef NDEBUG
            for (ridx_t i = 0; i < red_data.partial_cover.size(); ++i) {
                assert(red_data.total_cover[i] > 0);
                assert(red_data.partial_cover[i] > 0);
                assert(red_data.partial_cover[i] <= red_data.total_cover[i]);
            }
#endif
        }
    };

    template <size_t Depth>
    struct Enumerator {
        static void invoke(Instance const& inst,
                           RedundancyData& red_data,
                           bool (&vars)[CFT_ENUM_VARS],
                           bool (&sol)[CFT_ENUM_VARS]) {

            fmt::print("{} ", Depth);

            auto& partial_cover = red_data.partial_cover;
            auto& total_cover   = red_data.total_cover;

#ifndef NDEBUG
            assert(red_data.partial_cov_count <= partial_cover.size());
            for (ridx_t i = 0; i < partial_cover.size(); ++i) {
                assert(total_cover[i] > 0);
                assert(partial_cover[i] <= total_cover[i]);
            }
#endif

            if (Depth == red_data.redund_set.size() ||
                red_data.partial_cov_count == partial_cover.size()) {
                Enumerator<CFT_ENUM_VARS>::invoke(inst, red_data, vars, sol);
                return;
            }

            cidx_t col_idx = red_data.redund_set[Depth].col;
            auto   col     = inst.cols[col_idx];

            assert(!partial_cover.is_redundant_cover(col) || total_cover.is_redundant_uncover(col));

            if (red_data.partial_cost + red_data.redund_set[Depth].cost < red_data.best_cost &&
                !partial_cover.is_redundant_cover(col)) {

                vars[Depth] = true;
                red_data.partial_cov_count += partial_cover.cover(col);
                red_data.partial_cost += red_data.redund_set[Depth].cost;

                Enumerator<Depth + 1>::invoke(inst, red_data, vars, sol);

                vars[Depth] = false;
                red_data.partial_cov_count -= partial_cover.uncover(col);
                red_data.partial_cost -= red_data.redund_set[Depth].cost;
            }

            if (total_cover.is_redundant_uncover(col)) {
                total_cover.uncover(col);
                Enumerator<Depth + 1>::invoke(inst, red_data, vars, sol);
                total_cover.cover(col);
            }
        }
    };
}  // namespace

// Remove redundant columns from the redundancy set using an implicit enumeration. NOTE: assumes no
// more than CFT_ENUM_VARS columns are redundant.
inline void enumeration_removal(RedundancyData& red_set, Instance const& inst) {
    assert(red_set.redund_set.size() <= CFT_ENUM_VARS);
    real_t old_ub = red_set.best_cost;
    if (red_set.partial_cost >= old_ub || red_set.redund_set.empty())
        return;

    // TODO(cava): Redundant set can be an instance like, with a subset of rows and columns
    bool curr_keep_state[CFT_ENUM_VARS] = {};
    bool cols_to_keep[CFT_ENUM_VARS]    = {};

    fmt::print("#E ");
    Enumerator<0>::invoke(inst, red_set, curr_keep_state, cols_to_keep);
    fmt::print("\n");

    if (red_set.best_cost < old_ub)
        // cols_to_keep is updated only if the upper bound is improved
        for (cidx_t r = 0; r < red_set.redund_set.size(); ++r)
            if (!cols_to_keep[r])
                red_set.cols_to_remove.push_back(red_set.redund_set[r].col);
}

// Remove redundant columns from the redundancy set using an heuristic greedy approach until
// CFT_ENUM_VARS columns are left.
inline void heuristic_removal(RedundancyData& red_set, Instance const& inst) {
    while (red_set.partial_cost < red_set.best_cost && red_set.redund_set.size() > CFT_ENUM_VARS) {

        if (red_set.partial_cov_count == red_set.partial_cover.size())
            return;

        cidx_t j = red_set.redund_set.back().col;
        red_set.redund_set.pop_back();
        red_set.total_cover.uncover(inst.cols[j]);
        red_set.cols_to_remove.push_back(j);

        // They say they update the redudant set after every removal, which has the only effect
        // of exting earlier (with more elements) and start the enumeration.
        // TODO(cava): test if it actually improve/degrade performace/quality
        remove_if(red_set.redund_set, [&](CidxAndCost x) {
            if (red_set.total_cover.is_redundant_uncover(inst.cols[x.col]))
                return false;
            red_set.partial_cost += inst.costs[x.col];
            red_set.partial_cov_count += red_set.partial_cover.cover(inst.cols[x.col]);
            return true;
        });
    }
}

}  // namespace cft

#endif /* CFT_SRC_GREEDY_REDUNDANCY_HPP */
