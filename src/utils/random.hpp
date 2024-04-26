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
        using type = Xoshiro256PP;
    };

    template <>
    struct prng_picker_impl<uint32_t> {
        using type = Xoshiro128PP;
    };

    template <>
    struct prng_picker_impl<float> {
        using type = Xoshiro128P;
    };

    template <>
    struct prng_picker_impl<double> {
        using type = Xoshiro256P;
    };
}  // namespace local

// Xoshiro library provides specialized PRNGs for different result types.
template <typename TargetT>
struct prng_picker : local::prng_picker_impl<native_t<TargetT>> {};

////////////////////////////////// UTILITY RANDOM FUNCTIONS //////////////////////////////////

// Generate a canonical uniform distribution in the [0,1) range (unbiased).
// The gist of it is to only take the mantissa bits +1 of the PRNG result and scale it to [0,1).
// Could use SFINAE here, but I preferred to rely on compiler optimizations for clarity.
template <typename FlT, typename RndT>
inline FlT canonical_gen(RndT& rnd) {
    using fl_type                  = native_t<FlT>;
    using gen_type                 = typename RndT::result_type;
    static constexpr size_t gen_sz = sizeof(gen_type);
    static constexpr bool   is_u32 = std::is_same<gen_type, uint32_t>::value;
    static constexpr bool   ge_f32 = std::is_floating_point<fl_type>::value && sizeof(fl_type) >= 4;
    static constexpr bool   ge_f64 = std::is_floating_point<fl_type>::value && sizeof(fl_type) >= 8;
    static constexpr bool ge_f128 = std::is_floating_point<fl_type>::value && sizeof(fl_type) >= 16;

    static_assert(std::is_unsigned<gen_type>::value && gen_sz >= 4,
                  "PRNG results type is not a 32 or 64 bit unsigned type.");
    static_assert(ge_f32, "Floating point type not supported.");

    fl_type result = {};
    if (ge_f128) {
        static constexpr auto factor = 2.0L * checked_cast<fl_type>(1ULL << (gen_sz * 8U - 1U));

        result = checked_cast<fl_type>(rnd()) / factor;
    } else if (ge_f64) {
        static constexpr auto get_mantis_shift = is_u32 ? 0U : 11U;
        static constexpr auto factor = checked_cast<fl_type>(1ULL << (is_u32 ? 32ULL : 53ULL));

        result = checked_cast<fl_type>(rnd() >> get_mantis_shift) / factor;
    } else {
        assert(ge_f32);
        static constexpr auto get_mantis_shift = 8U * (gen_sz - 3U);  // to get usable bits

        result = checked_cast<fl_type>(rnd() >> get_mantis_shift) /
                 checked_cast<fl_type>(1U << 24U);
    }
    return FlT{result};
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
