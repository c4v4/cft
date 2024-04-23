// SPDX-FileCopyrightText: 2024 Francesco Cavaliere <francescocava95@gmail.com>
// SPDX-License-Identifier: MIT

#ifndef CAV_TEST_TEST_UTILS_HPP
#define CAV_TEST_TEST_UTILS_HPP

#include <algorithm>

#include "core/Instance.hpp"

namespace cft {

// Create a randomize instance with a number of columns between 10 and max_ncols + 10.
// The first 10 columns are always present to provide a bad solution that guarantees feasibility.
inline Instance make_easy_inst(uint64_t seed, cidx_t max_ncols) {

    auto cols = SparseBinMat<ridx_t>();
    cols.push_back({1_R, 2_R, 3_R, 4_R, 5_R, 6_R, 7_R, 8_R, 9_R, 10_R});
    cols.push_back({11_R, 12_R, 13_R, 14_R, 15_R, 16_R, 17_R, 18_R, 19_R, 20_R});
    cols.push_back({21_R, 22_R, 23_R, 24_R, 25_R, 26_R, 27_R, 28_R, 29_R, 30_R});
    cols.push_back({31_R, 32_R, 33_R, 34_R, 35_R, 36_R, 37_R, 38_R, 39_R, 40_R});
    cols.push_back({41_R, 42_R, 43_R, 44_R, 45_R, 46_R, 47_R, 48_R, 49_R, 50_R});
    cols.push_back({51_R, 52_R, 53_R, 54_R, 55_R, 56_R, 57_R, 58_R, 59_R, 60_R});
    cols.push_back({61_R, 62_R, 63_R, 64_R, 65_R, 66_R, 67_R, 68_R, 69_R, 70_R});
    cols.push_back({71_R, 72_R, 73_R, 74_R, 75_R, 76_R, 77_R, 78_R, 79_R, 80_R});
    cols.push_back({81_R, 82_R, 83_R, 84_R, 85_R, 86_R, 87_R, 88_R, 89_R, 90_R});
    cols.push_back({91_R, 92_R, 93_R, 94_R, 95_R, 96_R, 97_R, 98_R, 99_R, 0_R});
    auto costs = std::vector<real_t>(
        {100.0_F, 100.0_F, 100.0_F, 100.0_F, 100.0_F, 100.0_F, 100.0_F, 100.0_F, 100.0_F, 100.0_F});

    auto rows = std::vector<ridx_t>{};
    for (ridx_t i = 0_R; i < 100_R; ++i)
        rows.push_back(i);

    auto   rnd      = prng_t{seed};
    cidx_t num_cols = roll_dice(rnd, 0_C, max_ncols);
    for (cidx_t i = 0_C; i < num_cols; ++i) {
        std::shuffle(rows.begin(), rows.end(), rnd);
        size_t c_size = rnd() % 10;
        cols.push_back(make_span(rows.data(), c_size));
        costs.push_back(checked_cast<real_t>(roll_dice(rnd, 1, 3)));
    }

    auto inst    = Instance();
    inst.cols    = std::move(cols);
    inst.costs   = std::move(costs);
    ridx_t nrows = 100_R;
    fill_rows_from_cols(inst.cols, nrows, inst.rows);
    return inst;
}
}  // namespace cft

#endif /* CAV_TEST_TEST_UTILS_HPP */
