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
    inline void compute_col_reduced_costs(Instance const&            inst,
                                          std::vector<real_t> const& lagr_mult,
                                          std::vector<real_t>&       reduced_costs) {
        for (cidx_t j = 0; j < inst.cols.size(); ++j) {
            reduced_costs[j] = inst.costs[j];
            for (ridx_t i : inst.cols[j])
                reduced_costs[j] -= lagr_mult[i];
        }
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

        auto heap = make_sorted_array<cidx_t, mincov>(
            [&reduced_costs](cidx_t i, cidx_t j) { return reduced_costs[i] < reduced_costs[j]; });

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
            core_inst.solcosts.push_back(limits<real_t>::max());
            core_inst.costs.push_back(inst.costs[j]);
        }
    }

}  // namespace

struct Pricer {

    // Caches.
    Sorter              sorter;
    std::vector<real_t> reduced_costs;
    std::vector<cidx_t> idxs;
    std::vector<bool>   taken_idxs;

    void operator()(Instance const&            inst,
                    std::vector<real_t> const& lagr_mult,
                    Instance&                  core_inst) {

        ridx_t const nrows = inst.rows.size();
        cidx_t const ncols = inst.cols.size();

        _prepare_caches(ncols);

        compute_col_reduced_costs(inst, lagr_mult, reduced_costs);
        select_c1_col_idxs(inst, sorter, reduced_costs, 5 * nrows, idxs, taken_idxs);
        select_c2_col_idxs(inst, reduced_costs, idxs, taken_idxs);

        init_partial_instance(inst, idxs, core_inst);
        complete_init(core_inst, nrows);
    }

private:
    void _prepare_caches(ridx_t ncols) {
        reduced_costs.resize(ncols);
        taken_idxs.assign(ncols, false);
        idxs.clear();
    }
};

inline Pricer make_pricer() {
    return Pricer{make_sorter(), {}, {}, {}};
}

}  // namespace cft

#endif