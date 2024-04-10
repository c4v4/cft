// Copyright (c) 2024 Francesco Cavaliere
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>.

#ifndef CFT_SRC_SUBGRADIENT_PRICER_HPP
#define CFT_SRC_SUBGRADIENT_PRICER_HPP


#include <vector>

#include "core/Instance.hpp"
#include "core/cft.hpp"
#include "utils/SortedArray.hpp"
#include "utils/sort.hpp"

constexpr int mincov = 5;

namespace cft {

class Pricer {
    // Caches.
    Sorter              sorter;
    std::vector<real_t> reduced_costs;
    std::vector<bool>   taken_idxs;

public:
    real_t operator()(Instance const&            inst,
                      std::vector<real_t> const& lagr_mult,
                      InstAndMap&                core) {

        assert(!inst.cols.empty());
        assert(!core.inst.cols.empty());

        ridx_t nrows = inst.rows.size();
        cidx_t ncols = inst.cols.size();

        core.col_map.clear();
        _prepare_caches(ncols, reduced_costs, taken_idxs);

        auto real_lower_bound = _compute_col_reduced_costs(inst, lagr_mult, reduced_costs);
        _select_c1_col_idxs(inst, sorter, reduced_costs, 5ULL * nrows, core.col_map, taken_idxs);
        _select_c2_col_idxs(inst, reduced_costs, core.col_map, taken_idxs);

        _init_partial_instance(inst, core.col_map, core.inst);
        fill_rows_from_cols(core.inst.cols, nrows, core.inst.rows);

        return real_lower_bound;
    }

private:
    static void _prepare_caches(ridx_t               ncols,
                                std::vector<real_t>& reduced_costs,
                                std::vector<bool>&   taken_idxs) {
        reduced_costs.resize(ncols);
        taken_idxs.assign(ncols, false);
    }

    static real_t _compute_col_reduced_costs(Instance const&            inst,
                                             std::vector<real_t> const& lagr_mult,
                                             std::vector<real_t>&       reduced_costs) {

        real_t real_lower_bound = 0.0;
        for (real_t u : lagr_mult)
            real_lower_bound += u;

        for (cidx_t j = 0; j < inst.cols.size(); ++j) {
            reduced_costs[j] = inst.costs[j];
            for (ridx_t i : inst.cols[j])
                reduced_costs[j] -= lagr_mult[i];

            if (reduced_costs[j] < 0.0)
                real_lower_bound += reduced_costs[j];
        }
        return real_lower_bound;
    }

    static void _select_c1_col_idxs(Instance const&            inst,
                                    Sorter&                    sorter,
                                    std::vector<real_t> const& reduced_costs,
                                    size_t                     maxsize,
                                    std::vector<cidx_t>&       idxs,
                                    std::vector<bool>&         taken_idxs) {
        assert(idxs.empty());

        for (cidx_t j = 0; j < inst.cols.size(); ++j)
            if (reduced_costs[j] < 0.1)
                idxs.push_back(j);

        if (idxs.size() > maxsize) {
            sorter.nth_element(idxs, maxsize - 1, [&](cidx_t i) { return reduced_costs[i]; });
            idxs.resize(maxsize);
        }

        for (cidx_t j : idxs)
            taken_idxs[j] = true;
    }

    static void _select_c2_col_idxs(Instance const&            inst,
                                    std::vector<real_t> const& reduced_costs,
                                    std::vector<cidx_t>&       idxs,
                                    std::vector<bool>&         taken_idxs) {

        ridx_t const nrows = inst.rows.size();

        auto heap = make_custom_compare_sorted_array<cidx_t, mincov>(
            [&](cidx_t i, cidx_t j) { return reduced_costs[i] < reduced_costs[j]; });

        for (ridx_t i = 0; i < nrows; ++i) {
            heap.clear();
            for (cidx_t const j : inst.rows[i])
                heap.try_insert(j);
            for (cidx_t j : heap) {
                if (!taken_idxs[j]) {
                    taken_idxs[j] = true;
                    idxs.push_back(j);
                }
            }
        }
    }

    static void _init_partial_instance(Instance const&            inst,
                                       std::vector<cidx_t> const& idxs,
                                       Instance&                  core_inst) {

        // Clean up the current core instance.
        core_inst.cols.clear();
        core_inst.rows.clear();
        core_inst.costs.clear();
        core_inst.solcosts.clear();
        for (cidx_t j : idxs)
            push_back_col_from(inst, j, core_inst);  // Add column to core_inst
    }
};


}  // namespace cft


#endif /* CFT_SRC_SUBGRADIENT_PRICER_HPP */
