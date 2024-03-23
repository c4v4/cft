#ifndef CFT_INCLUDE_PRICER_HPP
#define CFT_INCLUDE_PRICER_HPP

#include <vector>

#include "core/SortedArray.hpp"
#include "core/cft.hpp"
#include "core/sort.hpp"
#include "instance/Instance.hpp"

constexpr int mincov = 5;

namespace cft {
namespace {
    inline real_t compute_col_reduced_costs(Instance const&            inst,
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

    inline void select_c1_col_idxs(Instance const&            inst,
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

    inline void select_c2_col_idxs(Instance const&            inst,
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

    inline void init_partial_instance(Instance const&            inst,
                                      std::vector<cidx_t> const& idxs,
                                      Instance&                  core_inst) {

        // Clean up the current core instance.
        core_inst.cols.clear();
        core_inst.rows.clear();
        core_inst.costs.clear();
        core_inst.solcosts.clear();

        for (cidx_t j : idxs) {
            core_inst.cols.push_back(inst.cols[j]);
            core_inst.solcosts.push_back(inst.solcosts[j]);
            core_inst.costs.push_back(inst.costs[j]);
        }
    }

}  // namespace

struct Pricer {

    // Caches.
    Sorter              sorter;
    std::vector<real_t> reduced_costs;
    std::vector<bool>   taken_idxs;

    real_t operator()(Instance const&            inst,
                      std::vector<real_t> const& lagr_mult,
                      InstAndMap&                core) {

        assert(!inst.cols.empty());
        assert(!core.inst.cols.empty());

        ridx_t nrows = inst.rows.size();
        cidx_t ncols = inst.cols.size();

        core.col_map.clear();
        _prepare_caches(ncols);

        auto real_lower_bound = compute_col_reduced_costs(inst, lagr_mult, reduced_costs);
        select_c1_col_idxs(inst, sorter, reduced_costs, 5 * nrows, core.col_map, taken_idxs);
        select_c2_col_idxs(inst, reduced_costs, core.col_map, taken_idxs);

        init_partial_instance(inst, core.col_map, core.inst);
        fill_rows_from_cols(core.inst.cols, nrows, core.inst.rows);

        return real_lower_bound;
    }

private:
    void _prepare_caches(ridx_t ncols) {
        reduced_costs.resize(ncols);
        taken_idxs.assign(ncols, false);
    }
};


}  // namespace cft

#endif