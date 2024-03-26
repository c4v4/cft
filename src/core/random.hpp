// Copyright (c) 2023 Francesco Cftaliere
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

#ifndef CFT_SRC_CORE_RANDOM_HPP
#define CFT_SRC_CORE_RANDOM_HPP

#include <cassert>

#include "core/xoshiro_prng.hpp"

namespace cft {

using prng_int32_t  = random::Xoshiro128PlusPlus;  // generic
using prng_int64_t  = random::Xoshiro256PlusPlus;  // generic
using prng_float_t  = random::Xoshiro128Plus;      // specialized for real distributions
using prng_double_t = random::Xoshiro256Plus;      // specialized for real distributions

using prng_t = prng_int64_t;  // default

inline bool coin_flip(prng_t& rnd, double true_prob = 0.5) noexcept {
    assert(0 <= true_prob && true_prob <= 1);
    using result_type = typename prng_t::result_type;
    auto treshold_val = static_cast<result_type>(static_cast<double>(true_prob) *
                                                 static_cast<double>(prng_t::max()));
    return rnd() <= treshold_val;
}

// Two coins flip mapped into one interval
// Let's consider the [0,1] interval.
// We want with prob P to get 1 for both coin flips
// We have 4 scenarios with the following probabilities:
// - toss 1 true: P
// - toss 2 true: P
// - toss 1&2 true: P^2
// - toss 1&2 false: 1 - 2P + P^2
///
// Which can be arranged as:
// [0------------------------------------------1]
//  |------ P ------|
//        |------ P ------|
//        |-- P^2 --|     |-- (1 - 2P + P^2) --|
///
// Splitting the [0,1] range into 4 subranges with the proper sizes.
///
inline std::array<bool, 2> two_coin_flips(prng_t& rnd, double true_p = 0.5) noexcept {

    assert(0 <= true_p && true_p <= 1);
    using result_type                    = typename prng_t::result_type;
    static constexpr result_type res_max = prng_t::max();

    auto p           = static_cast<long double>(true_p);
    auto range_width = static_cast<result_type>(p * res_max);

    // First interval
    // static constexpr res_type beg1 = zero;
    result_type end1 = range_width;

    // Second interval
    auto beg2 = static_cast<result_type>((p - p * p) * res_max);
    auto end2 = beg2 + range_width;

    auto rnd_val = rnd();
    return {rnd_val <= end1, beg2 <= rnd_val && rnd_val <= end2};
}

template <typename FlT>
inline FlT rnd_real(prng_t& rnd, FlT min, FlT max) noexcept {
    static constexpr auto rnd_range = static_cast<double>(prng_t::max() - prng_t::min());
    double                scale     = (max - min) / rnd_range;
    return min + scale * static_cast<double>(rnd());
}

template <typename IntT>
inline int roll_dice(prng_t& rnd, IntT min, IntT max) noexcept {
    assert(max - min >= 0);
    int result = min + static_cast<int>(rnd() % (max - min + 1));
    assert(min <= result && result <= max);
    return result;
}

}  // namespace cft

#endif /* CFT_SRC_CORE_RANDOM_HPP */
