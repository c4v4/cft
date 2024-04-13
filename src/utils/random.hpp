// Copyright (c) 2024 Francesco Cftaliere
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


#include "utils/assert.hpp"  // IWYU pragma:  keep
#include "utils/xoshiro_prng.hpp"

namespace cft {
template <typename TargetT = uint64_t>
struct prng_picker {
    using type = Xoshiro256PlusPlus;
};

template <>
struct prng_picker<uint32_t> {
    using type = Xoshiro128PlusPlus;
};

template <>
struct prng_picker<float> {
    using type = Xoshiro128Plus;
};

template <>
struct prng_picker<double> {
    using type = Xoshiro256Plus;
};

////////////////////////////////// UTILITY RANDOM FUNCTIONS //////////////////////////////////

// Generate a canonical uniform distribution in the [0,1) range (unbiased)
template <typename FlT, typename RndT>
inline FlT canonical_gen(RndT& rnd) {
    using gen_type               = typename RndT::result_type;
    static constexpr bool is_u32 = std::is_same<gen_type, uint32_t>::value;
    static constexpr bool is_u64 = std::is_unsigned<gen_type>::value && sizeof(gen_type) == 8;
    static constexpr bool is_f32 = std::is_same<FlT, float>::value;
    static constexpr bool is_f64 = std::is_same<FlT, double>::value;

    static_assert(is_u32 || is_u64, "PRNG results type is not a 32 or 64 bit unsigned type.");
    static_assert(is_f32 || is_f64, "RetT type is not a 32 or 64 bit floating point type.");

    static constexpr float  f32_mantis_mult = 5.960464477539063e-08;                        // 2^-24
    static constexpr double f64_mantis_mult = 1.1102230246251565404236316680908203125e-16;  // 2^-53
    static constexpr uint32_t f32_exp_size  = 8;
    static constexpr uint32_t f64_exp_size  = 11;

    if (is_u32 && is_f32)
        return static_cast<FlT>(rnd() >> f32_exp_size) * f32_mantis_mult;  // u32 -> f32
    if (is_u32 && is_f64)
        return static_cast<FlT>(rnd()) * 2.3283064365386962890625e-10;  // 2^-32  // u32 -> f64
    if (is_u64 && is_f32)
        return static_cast<FlT>(rnd() >> (32U + f32_exp_size)) * f32_mantis_mult;  // u64 -> f32
    return static_cast<FlT>(rnd() >> f64_exp_size) * f64_mantis_mult;              // u64 -> f64
}

// Generate a random real number in the [min, max) range
template <typename FlT, typename RndT>
inline FlT rnd_real(RndT& rnd, FlT min, FlT max) {
    static constexpr auto prng_range = static_cast<double>(RndT::max() - RndT::min());
    double                scale      = (max - min) / prng_range;
    return min + scale * static_cast<double>(rnd() - RndT::min());
}

// Generate a random integer in the [min, max] range
template <typename IntT, typename RndT>
inline IntT roll_dice(RndT& rnd, IntT min, IntT max) {
    static_assert(std::is_unsigned<typename RndT::result_type>::value,
                  "PRNG result is not unsigned.");
    assert(double(max - min) <= double(RndT::max() - RndT::min()) && "Range too large.");

    assert(max - min >= 0);
    int result = min + static_cast<int>(rnd() % (max - min + 1));
    assert(min <= result && result <= max);
    return result;
}

// Return true with probability `true_prob`
template <typename RndT>
inline bool coin_flip(RndT& rnd, double true_prob = 0.5) {
    assert(0 <= true_prob && true_prob <= 1);
    return canonical_gen<double>(rnd) <= true_prob;
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
template <typename RndT>
inline std::array<bool, 2> two_coin_flips(RndT& rnd, double true_p = 0.5) {
    static_assert(RndT::min() == 0, "PRNG min value is not 0.");

    assert(0 <= true_p && true_p <= 1);
    using result_type                    = typename RndT::result_type;
    static constexpr result_type res_max = RndT::max();

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

}  // namespace cft


#endif /* CFT_SRC_CORE_RANDOM_HPP */
