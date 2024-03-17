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

#ifndef CFT_SRC_GREEDY_REDUNDANCYSET_HPP
#define CFT_SRC_GREEDY_REDUNDANCYSET_HPP

#include "core/cft.hpp"
#include "core/coverage.hpp"
#include "core/limits.hpp"

namespace cft {

struct CidxAndCost {  // TODO(cava): if used elsewhere, can be move in core/
    cidx_t col;
    real_t cost;
};

/// @brief Data structure to store the redundancy set and related information
struct RedundancyData {
    std::vector<CidxAndCost> redund_set;      // redundant columns + their cost
    CoverCounters<>          total_cover;     // row-cov if all the remaining columns are selected
    CoverCounters<>          partial_cover;   // row-cov if we selected the current column
    std::vector<cidx_t>      cols_to_remove;  // list of columns to remove
    real_t                   best_cost;       // current best upper bound
    real_t                   partial_cost;    // current solution cost
    cidx_t                   partial_cov_count;
};

inline RedundancyData make_redundancy_data(ridx_t nrows = 0) {
    return {{},
            make_cover_counters(nrows),
            make_cover_counters(nrows),
            {},
            limits<real_t>::max(),
            0.0,
            0};
}

}  // namespace cft

#endif /* CFT_SRC_GREEDY_REDUNDANCYSET_HPP */
