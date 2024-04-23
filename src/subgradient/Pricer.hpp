// SPDX-FileCopyrightText: 2024 Francesco Cavaliere <francescocava95@gmail.com>
// SPDX-License-Identifier: MIT

#ifndef CFT_SRC_SUBGRADIENT_PRICER_HPP
#define CFT_SRC_SUBGRADIENT_PRICER_HPP


#include <vector>

#include "core/Instance.hpp"
#include "core/cft.hpp"
#include "utils/SortedArray.hpp"
#include "utils/sort.hpp"

namespace cft {

class Pricer {
    static constexpr int mincov = 5;

    // Caches.
    std::vector<real_t> reduced_costs;
    std::vector<bool>   taken_idxs;

public:
    real_t operator()(Instance const&            inst,       // in
                      std::vector<real_t> const& lagr_mult,  // in
                      InstAndMap&                core        // out
    ) {
        ridx_t const nrows = rsize(inst.rows);
        cidx_t const ncols = csize(inst.cols);

        assert(nrows == rsize(lagr_mult));
        if (nrows == 0_R || ncols == 0_C)
            return 0.0_F;

        core.col_map.clear();
        taken_idxs.assign(ncols, false);

        auto real_lower_bound = _compute_col_reduced_costs(inst, lagr_mult, reduced_costs);
        _select_c1_col_idxs(inst, reduced_costs, core.col_map, taken_idxs);
        _select_c2_col_idxs(inst, reduced_costs, core.col_map, taken_idxs);

        _init_partial_instance(inst, core.col_map, core.inst);
        fill_rows_from_cols(core.inst.cols, nrows, core.inst.rows);

        return real_lower_bound;
    }

private:
    static real_t _compute_col_reduced_costs(Instance const&            inst,          // in
                                             std::vector<real_t> const& lagr_mult,     // in
                                             std::vector<real_t>&       reduced_costs  // out
    ) {
        real_t real_lower_bound = 0.0_F;
        for (real_t u : lagr_mult)
            real_lower_bound += u;

        reduced_costs.resize(csize(inst.cols));
        for (cidx_t j = 0_C; j < csize(inst.cols); ++j) {
            reduced_costs[j] = inst.costs[j];
            for (ridx_t i : inst.cols[j])
                reduced_costs[j] -= lagr_mult[i];

            if (reduced_costs[j] < 0.0_F)
                real_lower_bound += reduced_costs[j];
        }
        return real_lower_bound;
    }

    static void _select_c1_col_idxs(Instance const&            inst,           // in
                                    std::vector<real_t> const& reduced_costs,  // in
                                    std::vector<cidx_t>&       idxs,           // inout
                                    std::vector<bool>&         taken_idxs      // inout
    ) {
        assert(idxs.empty());

        for (cidx_t j = 0_C; j < csize(inst.cols); ++j)
            if (reduced_costs[j] < 0.1_F)
                idxs.push_back(j);

        cidx_t const maxsize = as_cidx(5_R * rsize(inst.rows));
        if (csize(idxs) > maxsize) {
            cft::nth_element(idxs, maxsize - 1_C, [&](cidx_t i) { return reduced_costs[i]; });
            idxs.resize(maxsize);
        }

        for (cidx_t j : idxs)
            taken_idxs[j] = true;
    }

    static void _select_c2_col_idxs(Instance const&            inst,           // in
                                    std::vector<real_t> const& reduced_costs,  // in
                                    std::vector<cidx_t>&       idxs,           // inout
                                    std::vector<bool>&         taken_idxs      // inout
    ) {
        ridx_t const nrows = rsize(inst.rows);

        auto heap = make_custom_key_sorted_array<cidx_t, mincov>(
            [&](cidx_t j) { return reduced_costs[j]; });

        for (ridx_t i = 0_R; i < nrows; ++i) {
            heap.clear();
            for (cidx_t j : inst.rows[i])
                heap.try_insert(j);
            for (cidx_t j : heap) {
                if (!taken_idxs[j]) {
                    taken_idxs[j] = true;
                    idxs.push_back(j);
                }
            }
        }
    }

    static void _init_partial_instance(Instance const&            inst,      // in
                                       std::vector<cidx_t> const& idxs,      // in
                                       Instance&                  core_inst  // inout
    ) {
        // Clean up the current core instance.
        core_inst.cols.clear();
        core_inst.rows.clear();
        core_inst.costs.clear();
        for (cidx_t j : idxs)
            push_back_col_from(inst, j, core_inst);  // Add column to core_inst
    }
};


}  // namespace cft


#endif /* CFT_SRC_SUBGRADIENT_PRICER_HPP */
