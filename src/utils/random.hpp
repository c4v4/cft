// SPDX-FileCopyrightText: 2024 Francesco Cavaliere <francescocava95@gmail.com>
// SPDX-License-Identifier: MIT

#ifndef CFT_SRC_CORE_RANDOM_HPP
#define CFT_SRC_CORE_RANDOM_HPP


#include "utils/assert.hpp"  // IWYU pragma:  keep
#include "utils/utility.hpp"
#include "utils/xoshiro_prng.hpp"

namespace cft {

// Xoshiro library provides specialized PRNGs for different result types.
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

// Generate a canonical uniform distribution in the [0,1) range (unbiased).
// The gist of it is to only take the mantissa bits +1 of the PRNG result and scale it to [0,1).
// Could use SFINAE here, but I preferred to rely o ncompiler optimizations for clarity.
template <typename FlT, typename RndT>
inline FlT canonical_gen(RndT& rnd) {
    using gen_type               = typename RndT::result_type;
    static constexpr bool is_u32 = std::is_same<gen_type, uint32_t>::value;
    static constexpr bool is_u64 = sizeof(gen_type) == 8 && std::is_unsigned<gen_type>::value;
    static constexpr bool is_f32 = std::is_same<FlT, float>::value;
    static constexpr bool is_f64 = std::is_same<FlT, double>::value;

    static_assert(is_u32 || is_u64, "PRNG results type is not a 32 or 64 bit unsigned type.");
    static_assert(is_f32 || is_f64, "RetT type is not a 32 or 64 bit floating point type.");

    static constexpr float  f32_mantis_mult = 5.960464477539063e-08F;                       // 2^-24
    static constexpr double f64_mantis_mult = 1.1102230246251565404236316680908203125e-16;  // 2^-53
    static constexpr uint64_t f32_exp_size  = 8;
    static constexpr uint64_t f64_exp_size  = 11;

    if (is_u32 && is_f32)
        return static_cast<FlT>(rnd() >> f32_exp_size) * f32_mantis_mult;  // u32 -> f32
    if (is_u32 && is_f64)
        return static_cast<FlT>(rnd()) * 2.3283064365386962890625e-10;  // 2^-32  // u32 -> f64
    if (is_u64 && is_f32)
        return static_cast<FlT>(rnd() >> (32U + f32_exp_size)) * f32_mantis_mult;  // u64 -> f32
    assert(is_u64 && is_f64);
    return static_cast<FlT>(rnd() >> f64_exp_size) * f64_mantis_mult;  // u64 -> f64
}

// Generate a random real number in the [min, max) range
template <typename FlT, typename RndT>
inline FlT rnd_real(RndT& rnd, FlT min, FlT max) {
    return min + (max - min) * canonical_gen<FlT>(rnd);
}

// Generate a random integer in the [min, max] range
template <typename IntT, typename RndT>
inline IntT roll_dice(RndT& rnd, IntT min, IntT max) {
    using gen_type = typename RndT::result_type;
    static_assert(std::is_unsigned<gen_type>::value, "PRNG result is not unsigned.");
    assert(max >= min);

    IntT result = min + checked_cast<IntT>(rnd() % checked_cast<gen_type>(max - min + IntT{1}));
    assert(min <= result && result <= max);
    return result;
}

// Return true with probability `true_prob`
template <typename RndT>
inline bool coin_flip(RndT& rnd, double true_prob = 0.5) {
    assert(0 <= true_prob && true_prob <= 1);
    return canonical_gen<double>(rnd) <= true_prob;
}

}  // namespace cft


#endif /* CFT_SRC_CORE_RANDOM_HPP */
