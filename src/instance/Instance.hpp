#ifndef CFT_INCLUDE_INSTANCE_HPP
#define CFT_INCLUDE_INSTANCE_HPP


#include "core/SparseBinMat.hpp"
#include "core/cft.hpp"
#include "instance/parsing.hpp"

#define REMOVED_INDEX (cft::limits<cidx_t>::max())

namespace cft {


/// @brief `IdxMaps` tracks index mappings for a new Instance. It is used to maintain a
/// local-to-global mapping with the original instance when stored within Instance, and to
/// communicate old-to-new mappings when part of the instance is fixed.
/// An empty map signifies no changes, acting as an identity mapping.
struct IdxMaps {
    std::vector<cidx_t> col_idxs;
    std::vector<ridx_t> row_idxs;

    void clear() {
        col_idxs.clear();
        row_idxs.clear();
    }
};

inline IdxMaps make_idx_maps() {
    return {};
}

inline IdxMaps make_idx_maps(cidx_t ncols, ridx_t nrows) {
    auto idx_maps = IdxMaps{std::vector<cidx_t>(ncols), std::vector<ridx_t>(nrows)};
    for (cidx_t j = 0; j < ncols; ++j)
        idx_maps.col_idxs[j] = j;
    for (ridx_t i = 0; i < nrows; ++i)
        idx_maps.row_idxs[i] = i;

    return idx_maps;
}

/// @brief A data structure representing an instance using sparse binary matrix representation.
/// It maintains mappings with the original instance, tracking fixed costs and fixed indexes.
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

#ifndef NDEBUG
    void invariants_check() const {
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
#endif

    /// @brief Modifies instance by fixing columns in-place.
    /// New indexes are always <= old ones, allowing in-place external data structure updates.
    /// Note: Column fixing is irreversible, i.e., you cannot get the original instance from the
    /// subinstance.
    IdxMaps fix_columns(std::vector<cidx_t> const& cols_to_fix) {
        auto idx_maps = make_idx_maps();
        fix_columns(cols_to_fix, idx_maps);
        return idx_maps;
    }

    void fix_columns(std::vector<cidx_t> const& cols_to_fix, IdxMaps& idx_maps) {
        idx_maps.clear();
        if (cols_to_fix.empty())
            return;

        ridx_t removed_rows = _mark_and_update_fixed_elements(cols_to_fix);

        // If all rows were removed, clear everything
        if (removed_rows == rows.size()) {
            _set_inst_as_empty();
            return;
        }

        // Map old rows and columns to new ones based on whats has been marked
        _adjust_rows_pos_and_fill_map(idx_maps);
        _adjust_cols_pos_and_idxs_and_fill_map(idx_maps);
        _adjust_rows_idxs(idx_maps);
    }

private:
    /// @brief Mark columns and rows to be removed and update fixed cols and costs
    ridx_t _mark_and_update_fixed_elements(std::vector<cidx_t> const& cols_to_fix) {
        size_t removed_rows = 0;
        for (cidx_t lj : cols_to_fix) {
            cidx_t gj = orig_maps.col_idxs[lj];
            assert("Columns removed twice" && gj != REMOVED_INDEX);

            fixed_cost += costs[lj];                 // update fixed cost with new fixing
            fixed_orig_idxs.emplace_back(gj);        // add new fixed indexes
            orig_maps.col_idxs[lj] = REMOVED_INDEX;  // mark column to be removed
            for (ridx_t li : cols[lj]) {
                removed_rows += orig_maps.row_idxs[li] == REMOVED_INDEX ? 0 : 1;
                orig_maps.row_idxs[li] = REMOVED_INDEX;  // mark row to be removed
            }
        }
        return removed_rows;
    }

    void _set_inst_as_empty() {
        cols.clear();
        rows.clear();
        costs.clear();
        solcosts.clear();
        orig_maps.clear();
    }

    /// @brief Remove marked rows and make old->new row mapping
    void _adjust_rows_pos_and_fill_map(IdxMaps& idx_maps) {
        ridx_t old_nrows = rows.size();
        idx_maps.row_idxs.resize(old_nrows);
        ridx_t new_li = 0;
        for (ridx_t old_li = 0; old_li < old_nrows; ++old_li) {
            idx_maps.row_idxs[old_li] = new_li;
            if (orig_maps.row_idxs[old_li] != REMOVED_INDEX) {
                rows[new_li]               = std::move(rows[old_li]);
                orig_maps.row_idxs[new_li] = orig_maps.row_idxs[old_li];
                ++new_li;
            }
        }
        orig_maps.row_idxs.resize(new_li);
    }

    /// @brief Remove marked columns adjusting row indexes and make old->new col mapping
    void _adjust_cols_pos_and_idxs_and_fill_map(IdxMaps& idx_maps) {
        cidx_t old_ncols = cols.size();
        idx_maps.col_idxs.resize(old_ncols);
        cidx_t new_lj = 0;
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
            costs[new_lj]              = costs[old_lj];
            solcosts[new_lj]           = solcosts[old_lj];
            orig_maps.col_idxs[new_lj] = orig_maps.col_idxs[old_lj];
            ++new_lj;
        }
        orig_maps.col_idxs.resize(new_lj);
    }

    /// @brief Adjust column indexes stored in eanch row
    void _adjust_rows_idxs(IdxMaps const& idx_maps) {
        for (auto& row : rows)
            for (cidx_t& j : row)
                j = idx_maps.col_idxs[j];
    }
};

namespace {
    /// @brief Complete instance initialization by creating rows and orig_maps
    inline void complete_init(Instance& partial_inst, ridx_t nrows) {
        partial_inst.rows            = std::vector<std::vector<cidx_t>>(nrows);
        partial_inst.orig_maps       = make_idx_maps(partial_inst.cols.size(), nrows);
        partial_inst.fixed_orig_idxs = {};
        partial_inst.fixed_cost      = {};

        for (cidx_t j = 0; j < partial_inst.cols.size(); ++j)
            for (ridx_t i : partial_inst.cols[j])
                partial_inst.rows[i].push_back(j);
    }
}  // namespace

inline Instance make_instance(InstanceData&& inst_data) {
    Instance inst = {};
    inst.cols     = std::move(inst_data.cols);
    inst.costs    = std::move(inst_data.costs);
    inst.solcosts = std::move(inst_data.solcosts);

    complete_init(inst, inst_data.nrows);
    IF_DEBUG(inst.invariants_check());

    return inst;
}

inline Instance make_instance(InstanceData const& inst_data) {
    Instance inst = {};
    inst.cols     = inst_data.cols;
    inst.costs    = inst_data.costs;
    inst.solcosts = inst_data.solcosts;

    complete_init(inst, inst_data.nrows);
    IF_DEBUG(inst.invariants_check());

    return inst;
}

}  // namespace cft

#endif /* CFT_INCLUDE_INSTANCE_HPP */
