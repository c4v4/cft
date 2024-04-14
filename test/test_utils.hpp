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

#ifndef CAV_TEST_TEST_UTILS_HPP
#define CAV_TEST_TEST_UTILS_HPP

#include <algorithm>

#include "core/Instance.hpp"

namespace cft {

// Create a randomize instance with a number of columns between 10 and max_ncols + 10.
// The first 10 columns are always present to provide a bad solution that guarantees feasibility.
inline Instance make_easy_inst(uint64_t seed, cidx_t max_ncols) {

    auto cols = SparseBinMat<ridx_t>();
    cols.push_back({1, 2, 3, 4, 5, 6, 7, 8, 9, 10});
    cols.push_back({11, 12, 13, 14, 15, 16, 17, 18, 19, 20});
    cols.push_back({21, 22, 23, 24, 25, 26, 27, 28, 29, 30});
    cols.push_back({31, 32, 33, 34, 35, 36, 37, 38, 39, 40});
    cols.push_back({41, 42, 43, 44, 45, 46, 47, 48, 49, 50});
    cols.push_back({51, 52, 53, 54, 55, 56, 57, 58, 59, 60});
    cols.push_back({61, 62, 63, 64, 65, 66, 67, 68, 69, 70});
    cols.push_back({71, 72, 73, 74, 75, 76, 77, 78, 79, 80});
    cols.push_back({81, 82, 83, 84, 85, 86, 87, 88, 89, 90});
    cols.push_back({91, 92, 93, 94, 95, 96, 97, 98, 99, 0});
    auto costs = std::vector<real_t>{100, 100, 100, 100, 100, 100, 100, 100, 100, 100};

    auto rows = std::vector<ridx_t>{};
    for (ridx_t i = 0; i < 100; ++i)
        rows.push_back(i);

    auto rnd      = prng_t{seed};
    int  num_cols = roll_dice(rnd, 0, max_ncols);
    for (int i = 0; i < num_cols; ++i) {
        std::shuffle(rows.begin(), rows.end(), rnd);
        size_t c_size = rnd() % 10;
        cols.push_back(make_span(rows.data(), c_size));
        costs.push_back(checked_cast<real_t>(roll_dice(rnd, 1, 3)));
    }

    auto inst    = Instance();
    inst.cols    = std::move(cols);
    inst.costs   = std::move(costs);
    ridx_t nrows = 100;
    fill_rows_from_cols(inst.cols, nrows, inst.rows);
    return inst;
}
}  // namespace cft

#endif /* CAV_TEST_TEST_UTILS_HPP */
