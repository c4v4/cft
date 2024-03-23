#ifndef CFT_INCLUDE_INSTANCE_HPP
#define CFT_INCLUDE_INSTANCE_HPP


#include <cassert>
#include <vector>

#include "core/SparseBinMat.hpp"
#include "core/cft.hpp"
#include "core/sort.hpp"
#include "core/utility.hpp"

namespace cft {

// A data structure representing an instance using sparse binary matrix representation.
struct Instance {
    SparseBinMat<ridx_t>             cols;
    std::vector<std::vector<cidx_t>> rows;
    std::vector<real_t>              costs;
    std::vector<real_t>              solcosts;
};

#ifndef NDEBUG
inline void col_and_rows_check(SparseBinMat<ridx_t> const&             cols,
                               std::vector<std::vector<cidx_t>> const& rows) {
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

// Completes instance initialization by creating rows
inline void fill_rows_from_cols(SparseBinMat<ridx_t> const&       cols,
                                ridx_t                            nrows,
                                std::vector<std::vector<cidx_t>>& rows) {
    rows.resize(nrows);
    for (auto& row : rows) {
        row.clear();
        row.reserve(cols.idxs.size() / nrows);
    }

    for (cidx_t j = 0; j < cols.size(); ++j)
        for (ridx_t i : cols[j])
            rows[i].push_back(j);
    IF_DEBUG(col_and_rows_check(cols, rows));
}

struct InstAndMap {
    Instance            inst;
    std::vector<cidx_t> col_map;
};

inline InstAndMap build_tentative_core_instance(Instance const& inst,
                                                Sorter&         sorter,
                                                size_t          min_row_coverage) {
    ridx_t nrows        = inst.rows.size();
    auto   core_inst    = Instance{};
    auto   core_col_map = std::vector<cidx_t>();

    core_col_map.reserve(nrows * min_row_coverage);
    for (auto const& row : inst.rows)
        for (size_t n = 0; n < min(row.size(), min_row_coverage); ++n)
            core_col_map.push_back(row[n]);

    sorter.sort(core_col_map);
    cidx_t w      = 0;
    cidx_t prev_j = CFT_REMOVED_IDX;
    for (cidx_t j : core_col_map) {
        if (j == prev_j)
            continue;
        prev_j            = j;
        core_col_map[w++] = j;
        core_inst.cols.push_back(inst.cols[j]);
        core_inst.costs.push_back(inst.costs[j]);
        core_inst.solcosts.push_back(inst.solcosts[j]);
    }
    core_col_map.resize(w);

    fill_rows_from_cols(core_inst.cols, nrows, core_inst.rows);
    return {std::move(core_inst), std::move(core_col_map)};
}
}  // namespace cft

#endif /* CFT_INCLUDE_INSTANCE_HPP */
