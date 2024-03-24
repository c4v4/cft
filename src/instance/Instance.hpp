#ifndef CFT_INCLUDE_INSTANCE_HPP
#define CFT_INCLUDE_INSTANCE_HPP


#include <cassert>
#include <vector>

#include "core/SparseBinMat.hpp"
#include "core/cft.hpp"
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

}  // namespace cft

#endif /* CFT_INCLUDE_INSTANCE_HPP */
