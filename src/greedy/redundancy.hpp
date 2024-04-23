// SPDX-FileCopyrightText: 2024 Francesco Cavaliere <francescocava95@gmail.com>
// SPDX-License-Identifier: MIT

#ifndef CFT_SRC_GREEDY_REDUNDANCY_HPP
#define CFT_SRC_GREEDY_REDUNDANCY_HPP


#include "core/Instance.hpp"
#include "core/cft.hpp"
#include "utils/CoverCounters.hpp"
#include "utils/limits.hpp"
#include "utils/sort.hpp"
#include "utils/utility.hpp"


#define CFT_ENUM_VARS 10_C

namespace cft {
// Data structure to store the redundancy set and related information
struct RedundancyData {
    std::vector<CidxAndCost> redund_set;      // redundant columns + their cost
    CoverCounters            total_cover;     // row-cov if all the remaining columns are selected
    CoverCounters            partial_cover;   // row-cov if we selected the current column
    std::vector<cidx_t>      cols_to_remove;  // list of columns to remove
    real_t                   best_cost         = limits<real_t>::max();  // current best upper bound
    real_t                   partial_cost      = 0.0_F;                  // current solution cost
    ridx_t                   partial_cov_count = 0_R;                    // number of covered rows
};

#ifndef NDEBUG
inline void check_redundancy_data(Instance const&            inst,
                                  std::vector<cidx_t> const& sol,
                                  RedundancyData const&      red_set) {

    auto   total_check    = CoverCounters(rsize(inst.rows));
    auto   part_check     = CoverCounters(rsize(inst.rows));
    ridx_t part_cov_count = 0_R;
    for (cidx_t j : sol) {
        part_check.cover(inst.cols[j]);
        part_cov_count += as_ridx(total_check.cover(inst.cols[j]));
    }
    for (cidx_t j : red_set.cols_to_remove) {
        part_cov_count -= as_ridx(part_check.uncover(inst.cols[j]));
        total_check.uncover(inst.cols[j]);
    }
    for (CidxAndCost x : red_set.redund_set)
        part_cov_count -= as_ridx(part_check.uncover(inst.cols[x.idx]));

    assert(part_cov_count == red_set.partial_cov_count);
    for (ridx_t i = 0_R; i < rsize(inst.rows); ++i) {
        assert(red_set.total_cover[i] == total_check[i]);
        assert(red_set.partial_cover[i] == part_check[i]);
        assert(red_set.partial_cover[i] <= red_set.total_cover[i]);
    }
}
#endif

namespace local { namespace {

    // When the threshold is below a certain value, we can enumerate all possible
    // non-redundant combinations. As the threshold is known at compile-time, the enumeration
    // has a fixed maximum depth. Enumerator utilizes partial specialization (since `if
    // constexpr` is a C++17 feature) to limit the number of enumeration steps and (technically)
    // remove the recursion.
    template <size_t Cur>
    struct Enumerator;

    template <>
    struct Enumerator<CFT_ENUM_VARS> {
        static void invoke(Instance const& /*inst*/,           // unused
                           RedundancyData& red_data,           // inout
                           bool const (&vars)[CFT_ENUM_VARS],  // in
                           bool (&sol)[CFT_ENUM_VARS]          // out
        ) {
            if (red_data.partial_cost < red_data.best_cost) {
                red_data.best_cost = red_data.partial_cost;
                for (cidx_t s = 0_C; s < CFT_ENUM_VARS; ++s)
                    sol[s] = vars[s];
            }
        }
    };

    template <size_t Depth>
    struct Enumerator {
        static void invoke(Instance const& inst,         // in
                           RedundancyData& red_data,     // inout
                           bool (&vars)[CFT_ENUM_VARS],  // inout
                           bool (&sol)[CFT_ENUM_VARS]    // out
        ) {
            auto& partial_cover = red_data.partial_cover;
            auto& total_cover   = red_data.total_cover;

#ifndef NDEBUG
            assert(red_data.partial_cov_count <= rsize(partial_cover));
            for (ridx_t i = 0_R; i < rsize(partial_cover); ++i)
                assert(partial_cover[i] <= total_cover[i]);
#endif

            if (Depth == red_data.redund_set.size() ||
                red_data.partial_cov_count == rsize(partial_cover)) {
                Enumerator<CFT_ENUM_VARS>::invoke(inst, red_data, vars, sol);
                return;
            }

            cidx_t col_idx = red_data.redund_set[Depth].idx;
            auto   col     = inst.cols[col_idx];

            assert(!partial_cover.is_redundant_cover(col) || total_cover.is_redundant_uncover(col));

            if (red_data.partial_cost + red_data.redund_set[Depth].cost < red_data.best_cost &&
                !partial_cover.is_redundant_cover(col)) {

                vars[Depth] = true;
                red_data.partial_cov_count += as_ridx(partial_cover.cover(col));
                red_data.partial_cost += red_data.redund_set[Depth].cost;

                Enumerator<Depth + 1>::invoke(inst, red_data, vars, sol);

                vars[Depth] = false;
                red_data.partial_cov_count -= as_ridx(partial_cover.uncover(col));
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
}  // namespace local

inline void complete_init_redund_set(Instance const&            inst,         // in
                                     std::vector<cidx_t> const& sol,          // in
                                     real_t                     cutoff_cost,  // in
                                     RedundancyData&            red_data      // inout
) {
    red_data.redund_set.clear();
    red_data.partial_cover.reset(rsize(inst.rows));
    red_data.partial_cov_count = 0_R;
    red_data.cols_to_remove.clear();
    red_data.best_cost    = cutoff_cost;
    red_data.partial_cost = 0.0_F;

    for (cidx_t j : sol)
        if (red_data.total_cover.is_redundant_uncover(inst.cols[j]))
            red_data.redund_set.push_back({j, inst.costs[j]});
        else {
            red_data.partial_cov_count += as_ridx(red_data.partial_cover.cover(inst.cols[j]));
            red_data.partial_cost += inst.costs[j];
            if (red_data.partial_cost >= cutoff_cost)
                return;
        }
    cft::sort(red_data.redund_set, [](CidxAndCost x) { return x.cost; });
}

// Remove redundant columns from the redundancy set using an implicit enumeration. NOTE: assumes
// no more than CFT_ENUM_VARS columns are redundant.
inline void enumeration_removal(Instance const& inst,    // in
                                RedundancyData& red_set  // inout
) {
    assert(csize(red_set.redund_set) <= CFT_ENUM_VARS);
    real_t old_ub = red_set.best_cost;
    if (red_set.partial_cost >= old_ub || red_set.redund_set.empty())
        return;

    bool curr_keep_state[CFT_ENUM_VARS] = {};
    bool cols_to_keep[CFT_ENUM_VARS]    = {};

    local::Enumerator<0>::invoke(inst, red_set, curr_keep_state, cols_to_keep);

    if (red_set.best_cost < old_ub)
        for (cidx_t r = 0_C; r < csize(red_set.redund_set); ++r)
            if (!cols_to_keep[r])
                red_set.cols_to_remove.push_back(red_set.redund_set[r].idx);
}

// Remove redundant columns from the redundancy set using an heuristic greedy approach until
// CFT_ENUM_VARS columns are left.
inline void heuristic_removal(Instance const& inst,    // in
                              RedundancyData& red_set  // inout
) {
    while (red_set.partial_cost < red_set.best_cost && csize(red_set.redund_set) > CFT_ENUM_VARS) {

        if (red_set.partial_cov_count == rsize(inst.rows))
            return;

        cidx_t j = red_set.redund_set.back().idx;
        red_set.redund_set.pop_back();
        red_set.total_cover.uncover(inst.cols[j]);
        red_set.cols_to_remove.push_back(j);

        // They say they update the redudant set after every removal, which has the only effect
        // of exting earlier (with more elements) and start the enumeration.
        remove_if(red_set.redund_set, [&](CidxAndCost x) {
            if (red_set.total_cover.is_redundant_uncover(inst.cols[x.idx]))
                return false;
            red_set.partial_cost += inst.costs[x.idx];
            red_set.partial_cov_count += as_ridx(red_set.partial_cover.cover(inst.cols[x.idx]));
            return true;
        });
    }
}

}  // namespace cft


#endif /* CFT_SRC_GREEDY_REDUNDANCY_HPP */
