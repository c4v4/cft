// SPDX-FileCopyrightText: 2024 Francesco Cavaliere <francescocava95@gmail.com>
// SPDX-License-Identifier: MIT

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#define DOCTEST_CONFIG_SUPER_FAST_ASSERTS

#include <doctest/doctest.h>

#include "utils/random.hpp"

namespace cft {

TEST_CASE("test_canonical_gen") {
    auto rnd_u64    = prng_picker<uint64_t>::type(0);
    auto rnd_u32    = prng_picker<uint32_t>::type(1);
    auto rnd_float  = prng_picker<float>::type(2);
    auto rnd_double = prng_picker<double>::type(3);

    int over_half = 0;
    int total     = 0;
    for (int i = 0; i < 100000; i++) {
        // Test for u32 -> f32 conversion
        auto result1 = canonical_gen<float>(rnd_u32);
        CHECK(result1 >= 0.0F);
        CHECK(result1 < 1.0F);
        over_half += result1 > 0.5F ? 1 : 0;
        ++total;

        // Test for u32 -> f64 conversion
        auto result2 = canonical_gen<double>(rnd_float);
        CHECK(result2 >= 0.0);
        CHECK(result2 < 1.0);
        over_half += result2 > 0.5 ? 1 : 0;
        ++total;

        // Test for u64 -> f32 conversion
        auto result3 = canonical_gen<float>(rnd_u64);
        CHECK(result3 >= 0.0F);
        CHECK(result3 < 1.0F);
        over_half += result3 > 0.5F ? 1 : 0;
        ++total;

        // Test for u64 -> f64 conversion
        auto result4 = canonical_gen<double>(rnd_double);
        CHECK(result4 >= 0.0);
        CHECK(result4 < 1.0);
        over_half += result4 > 0.5 ? 1 : 0;
        ++total;

        // Test for u64 -> f128 conversion
        auto result5 = canonical_gen<long double>(rnd_double);
        CHECK(result5 >= 0.0L);
        CHECK(result5 < 1.0L);
        over_half += result5 > 0.5L ? 1 : 0;
        ++total;
    }
    double over_half_frac = over_half / static_cast<double>(total);
    CHECK(0.49 < over_half_frac);
    CHECK(over_half_frac < 0.51);
}

TEST_CASE("test_rnd_real") {
    auto rnd_u64    = prng_picker<uint64_t>::type(4);
    auto rnd_u32    = prng_picker<uint32_t>::type(5);
    auto rnd_float  = prng_picker<float>::type(6);
    auto rnd_double = prng_picker<double>::type(7);

    int over_half = 0;
    int total     = 0;
    for (int i = 0; i < 100000; i++) {
        {
            // Test for u32 -> f32 conversion
            float min = -10.0F * i, max = 10.0F * i + 1;
            auto  res = rnd_real<float>(rnd_u32, min, max);
            CHECK(res >= min);
            CHECK(res < max);
            over_half += res > (min + max) / 2.0F ? 1 : 0;
            ++total;
        }
        {
            // Test for u32 -> f64 conversion
            double min = -10.0 * i, max = 10.0 * i + 1;
            auto   res = rnd_real<double>(rnd_float, min, max);
            CHECK(res >= min);
            CHECK(res < max);
            over_half += res > (min + max) / 2.0 ? 1 : 0;
            ++total;
        }
        {
            // Test for u64 -> f32 conversion
            float min = -10.0F * i, max = 10.0F * i + 1;
            auto  res = rnd_real<float>(rnd_u64, min, max);
            CHECK(res >= min);
            CHECK(res < max);
            over_half += res > (min + max) / 2.0F ? 1 : 0;
            ++total;
        }
        {
            // Test for u64 -> f64 conversion
            double min = -10.0 * i, max = 10.0 * i + 1;
            auto   res = rnd_real<double>(rnd_double, min, max);
            CHECK(res >= min);
            CHECK(res < max);
            over_half += res > (min + max) / 2.0 ? 1 : 0;
            ++total;
        }
        {
            // Test for u64 -> f64 conversion
            long double min = -10.0L * i, max = 10.0L * i + 1;
            auto        res = rnd_real<long double>(rnd_double, min, max);
            CHECK(res >= min);
            CHECK(res < max);
            over_half += res > (min + max) / 2.0L ? 1 : 0;
            ++total;
        }
    }
    double over_half_frac = over_half / static_cast<double>(total);
    CHECK(0.49 < over_half_frac);
    CHECK(over_half_frac < 0.51);
}

TEST_CASE("test_roll_dice") {
    auto rnd_u64 = prng_picker<uint64_t>::type(8);
    auto rnd_u32 = prng_picker<uint32_t>::type(9);

    int over_half = 0;
    int total     = 0;
    for (int i = 0; i < 100000; i++) {
        {
            int32_t min = -10 * i, max = 10 * i + 1;
            auto    res = roll_dice<int32_t>(rnd_u32, min, max);
            CHECK(res >= min);
            CHECK(res <= max);
            over_half += res > (min + max) / 2 ? 1 : 0;
            ++total;
        }
        {
            int64_t min = -10L * i, max = 10L * i + 1;
            auto    res = roll_dice<int64_t>(rnd_u64, min, max);
            CHECK(res >= min);
            CHECK(res <= max);
            over_half += res > (min + max) / 2 ? 1 : 0;
            ++total;
        }
    }
    double over_half_frac = over_half / static_cast<double>(total);
    CHECK(0.49 < over_half_frac);
    CHECK(over_half_frac < 0.51);
}

TEST_CASE("test_coin_flip") {
    auto rnd_double = prng_picker<double>::type(10);
    auto rnd_float  = prng_picker<float>::type(11);

    int true_count = 0;
    int total      = 0;
    for (int i = 0; i < 100000; i++) {
        {
            auto true_p = canonical_gen<double>(rnd_float);
            bool res    = coin_flip(rnd_float, true_p);
            true_count += res ? 1 : 0;
            ++total;
        }
        {
            auto true_p = canonical_gen<double>(rnd_double);
            bool res    = coin_flip(rnd_double, true_p);
            true_count += res ? 1 : 0;
            ++total;
        }
    }
    double true_frac = true_count / static_cast<double>(total);
    CHECK(0.49 < true_frac);
    CHECK(true_frac < 0.51);
}


}  // namespace cft