// SPDX-FileCopyrightText: 2024 Francesco Cavaliere <francescocava95@gmail.com>
// SPDX-License-Identifier: MIT

#ifndef CFT_SRC_CORE_RANDOM_HPP
#define CFT_SRC_CORE_RANDOM_HPP


#include "utils/assert.hpp"  // IWYU pragma:  keep
#include "utils/custom_types.hpp"
#include "utils/utility.hpp"
#include "utils/xoshiro_prng.hpp"

namespace cft {

namespace local {
    template <typename T>
    struct prng_picker_impl {
        using type = Xoshiro256PlusPlus;
    };

    template <>
    struct prng_picker_impl<uint32_t> {
        using type = Xoshiro128PlusPlus;
    };

    template <>
    struct prng_picker_impl<float> {
        using type = Xoshiro128Plus;
    };

    template <>
    struct prng_picker_impl<double> {
        using type = Xoshiro256Plus;
    };
}  // namespace local

// Xoshiro library provides specialized PRNGs for different result types.
template <typename TargetT>
struct prng_picker : local::prng_picker_impl<native_t<TargetT>> {};

////////////////////////////////// UTILITY RANDOM FUNCTIONS //////////////////////////////////

// Generate a canonical uniform distribution in the [0,1) range (unbiased).
// The gist of it is to only take the mantissa bits +1 of the PRNG result and scale it to [0,1).
// Could use SFINAE here, but I preferred to rely o ncompiler optimizations for clarity.
template <typename FlT, typename RndT>
inline FlT canonical_gen(RndT& rnd) {
    using fl_type                   = native_t<FlT>;
    using gen_type                  = typename RndT::result_type;
    static constexpr size_t gen_sz  = sizeof(gen_type);
    static constexpr bool   is_u32  = std::is_same<gen_type, uint32_t>::value;
    static constexpr bool   is_u64  = gen_sz == 8 && std::is_unsigned<gen_type>::value;
    static constexpr bool   is_f32  = std::is_same<fl_type, float>::value;
    static constexpr bool   is_f64  = std::is_same<fl_type, double>::value;
    static constexpr bool   is_f128 = std::is_same<fl_type, long double>::value;

    static_assert(is_u32 || is_u64, "PRNG results type is not a 32 or 64 bit unsigned type.");
    static_assert(is_f32 || is_f64 || is_f128, "RetT type is not a floating point type.");

    auto res = fl_type{};
    if (is_f32) {
        static constexpr uint64_t get_mantis_shift = 8 * (gen_sz - 3);  // to get usable bits

        res = static_cast<fl_type>(rnd() >> get_mantis_shift) / checked_cast<double>(1U << 24U);
    } else if (is_f64) {
        static constexpr uint64_t get_mantis_shift = is_u32 ? 0 : 11;
        static constexpr auto     factor = checked_cast<double>(1ULL << (is_u32 ? 32ULL : 53ULL));

        res = static_cast<fl_type>(rnd() >> get_mantis_shift) / factor;
    } else {
        assert(is_f128);
        static constexpr auto factor = 2.0L * checked_cast<long double>(1ULL << (gen_sz * 8U - 1U));

        res = static_cast<fl_type>(rnd()) / factor;
    }
    return FlT{res};
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
