#ifndef CAV_INCLUDE_INSTANCE_HPPCFT_
#define CAV_INCLUDE_INSTANCE_HPPCFT_

#include <stdexcept>

#include "SparseBinMat.hpp"
#include "cft.hpp"
#include "parsing.hpp"
#include "util_functions.hpp"

#define REMOVED_INDEX (cft::limits<cidx_t>::max())

namespace cft {

/// @brief `IdxMaps` tracks index mappings for a new Instance. It is used to maintains a
/// local-to-global mapping with the original instance when stored within Instance, and to
/// communicates old-to-new mappings when part of the instance is fixed.
/// An empty map signifies no changes, acting as an identity mapping.
struct IdxMaps {
    std::vector<cidx_t> col_idxs;
    std::vector<ridx_t> row_idxs;
};

struct Instance {
    // Current instance data
    SparseBinMat<ridx_t>             cols;
    std::vector<std::vector<cidx_t>> rows;
    std::vector<real_t>              costs;
    std::vector<real_t>              solcosts;

    // Mappings with original instance
    IdxMaps             orig_maps;
    std::vector<cidx_t> fixed_orig_idxs;
    real_t              fixed_cost;

    // Candidate caches (uncomment on need):
    // mutable CoverCounters<>     cover_counters;
    // mutable CoverBits           cover_bits;
    // mutable std::vector<real_t> col_rcosts;

    inline void complete_init(ridx_t nrows) {
        rows               = std::vector<std::vector<cidx_t>>(nrows);
        orig_maps.col_idxs = std::vector<cidx_t>(cols.size());
        orig_maps.row_idxs = std::vector<ridx_t>(rows.size());
        fixed_orig_idxs    = {};
        fixed_cost         = {};

        for (cidx_t j = 0; j < cols.size(); ++j)
            for (ridx_t i : cols[j])
                rows[i].push_back(j);

        for (cidx_t j = 0; j < cols.size(); ++j)
            orig_maps.col_idxs[j] = j;

        for (ridx_t i = 0; i < rows.size(); ++i)
            orig_maps.row_idxs[i] = i;
    }

    inline void invariants_check() const {
        IF_DEBUG {
            for (cidx_t j = 0; j < cols.size(); ++j) {
                assert("Col is empty" && !cols[j].empty());
                assert("Col does not exist" && j < cols.size());
                for (ridx_t i : cols[j])
                    assert("Col not in row" && any(rows[i], [j](cidx_t rj) { return rj == j; }));
            }

            for (ridx_t i = 0; i < rows.size(); ++i) {
                assert("Row is empty" && !rows[i].empty());
                assert("Row does not exist" && i < rows.size());
                for (cidx_t j : rows[i])
                    assert("Row not in col" && any(cols[j], [i](cidx_t ci) { return ci == i; }));
            }
        }
    }

    /// @brief Modifies the instance size by fixing columns, creating a new subinstance in place.
    /// New indexes are always <= old ones, allowing in-place external data structure updates.
    /// Note: Column fixing is irreversible.
    IdxMaps fix_columns(std::vector<cidx_t>& cols_to_fix) {
        cidx_t old_ncols = cols.size(), old_nrows = rows.size();
        auto   idx_maps = IdxMaps{};

        if (cols_to_fix.empty())
            return idx_maps;

        // Mark columns and rows to be removed
        size_t removed_rows = 0;
        for (cidx_t lj : cols_to_fix) {
            cidx_t gj = orig_maps.col_idxs[lj];
            assert(gj != REMOVED_INDEX);

            fixed_cost += costs[lj];                 // update fixed cost with new fixing
            fixed_orig_idxs.emplace_back(gj);        // add new fixed indexes
            orig_maps.col_idxs[lj] = REMOVED_INDEX;  // mark column to be removed
            for (ridx_t li : cols[lj]) {
                removed_rows += orig_maps.row_idxs[li] == REMOVED_INDEX ? 0 : 1;
                orig_maps.row_idxs[li] = REMOVED_INDEX;  // mark row to be removed
            }
        }

        // If all rows were removed, clear everything
        if (removed_rows == old_nrows) {
            cols.clear();
            rows.clear();
            return idx_maps;
        }

        // Remove marked rows and make old->new row mapping
        ridx_t new_li     = 0;
        idx_maps.row_idxs = std::vector<ridx_t>(old_nrows);
        for (ridx_t old_li = 0; old_li < old_nrows; ++old_li) {
            idx_maps.row_idxs[old_li] = new_li;
            if (orig_maps.row_idxs[old_li] != REMOVED_INDEX) {
                rows[new_li]               = std::move(rows[old_li]);
                orig_maps.row_idxs[new_li] = orig_maps.row_idxs[old_li];
                ++new_li;
            }
        }
        orig_maps.row_idxs.resize(new_li);

        // Remove marked columns adjusting row indexes and make old->new col mapping
        cidx_t new_lj     = 0;
        idx_maps.col_idxs = std::vector<ridx_t>(old_ncols);
        for (ridx_t old_lj = 0; old_lj < old_ncols; ++old_lj) {
            idx_maps.col_idxs[old_lj] = new_lj;
            if (orig_maps.col_idxs[old_lj] == REMOVED_INDEX)
                continue;

            if (new_lj != old_lj) {  // move col forward to new position
                cols.begs[new_lj + 1] = cols.begs[new_lj] + cols[old_lj].size();
                size_t n = cols.begs[new_lj], o = cols.begs[old_lj];
                while (o < cols.begs[old_lj + 1])
                    cols.idxs[n++] = idx_maps.row_idxs[cols.idxs[o++]];
            }
            orig_maps.col_idxs[new_lj] = orig_maps.col_idxs[old_lj];
            ++new_lj;
        }
        orig_maps.col_idxs.resize(new_lj);

        // Adjust row indexes
        for (auto& row : rows)
            for (cidx_t& j : row)
                j = idx_maps.col_idxs[j];

        return idx_maps;
    }
};

inline Instance make_instance(InstanceData&& inst_data) {
    Instance inst = {};
    inst.cols     = std::move(inst_data.cols);
    inst.costs    = std::move(inst_data.costs);
    inst.solcosts = std::move(inst_data.solcosts);

    inst.complete_init(inst_data.nrows);
    IF_DEBUG inst.invariants_check();

    return inst;
}

inline Instance make_instance(InstanceData const& inst_data) {
    Instance inst = {};
    inst.cols     = inst_data.cols;
    inst.costs    = inst_data.costs;
    inst.solcosts = inst_data.solcosts;

    inst.complete_init(inst_data.nrows);
    IF_DEBUG inst.invariants_check();

    return inst;
}

}  // namespace cft

#endif /* CAV_INCLUDE_INSTANCE_HPPCFT_ */
