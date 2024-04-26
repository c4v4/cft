// SPDX-FileCopyrightText: 2024 Francesco Cavaliere <francescocava95@gmail.com>
// SPDX-License-Identifier: MIT

// Stripped-down Xoshiro pseudo random number generators based on David Blackman and Sebastiano Vigna's
// xoshiro generators (prng.di.unimi.it/)

#ifndef CFT_SRC_CORE_XOSHIRO_PRNG_HPP
#define CFT_SRC_CORE_XOSHIRO_PRNG_HPP

#include <cstdint>

#include "utils/limits.hpp"

namespace cft {
namespace random {
    namespace local { namespace {
        template <typename T>
        constexpr T rot_l(T x, T s) {
            return (x << s) | (x >> (8 * sizeof(x) - s));
        }

        template <uint64_t Shift, uint64_t Rot, typename T>
        void next_state(T (&state)[4]) {
            T t = state[1] << Shift;
            state[2] ^= state[0];
            state[3] ^= state[1];
            state[1] ^= state[2];
            state[0] ^= state[3];
            state[2] ^= t;
            state[3] = rot_l<T>(state[3], Rot);
        }
    }  // namespace
    }  // namespace local

    struct Xoshiro256PImpl {  // Original: http://prng.di.unimi.it/xoshiro256plus.c
        using result_type = uint64_t;

        static result_type next_rnd(result_type (&state)[4]) {
            result_type result = state[0] + state[3];
            local::next_state<17U, 45U>(state);
            return result;
        }
    };

    struct Xoshiro256PPImpl {  // Original: http://prng.di.unimi.it/xoshiro256plusplus.c
        using result_type = uint64_t;

        static result_type next_rnd(result_type (&state)[4]) {
            result_type result = local::rot_l<result_type>(state[0] + state[3], 23) + state[0];
            local::next_state<17U, 45U>(state);
            return result;
        }
    };

    struct Xoshiro128PImpl {  // Original: http://prng.di.unimi.it/xoshiro128plus.c
        using result_type = uint32_t;

        static result_type next_rnd(result_type (&state)[4]) {
            result_type result = state[0] + state[3];
            local::next_state<9U, 11U>(state);
            return result;
        }
    };

    struct Xoshiro128PPImpl {  // Original: http://prng.di.unimi.it/xoshiro128plusplus.c
        using result_type = uint32_t;

        static result_type next_rnd(result_type (&state)[4]) {
            result_type result = local::rot_l<result_type>(state[0] + state[3], 7) + state[0];
            local::next_state<9U, 11U>(state);
            return result;
        }
    };

    template <typename GenT>
    struct XoshiroMixIn : limits<typename GenT::result_type> {
        using result_type = typename GenT::result_type;
        result_type state[4];

        explicit XoshiroMixIn(uint64_t init_seed = 0ULL) {
            // SplitMix64
            for (result_type& seed : state) {
                uint64_t z = (init_seed += 0x9e3779b97f4a7c15);
                z          = (z ^ (z >> 30U)) * 0xbf58476d1ce4e5b9;
                z          = (z ^ (z >> 27U)) * 0x94d049bb133111eb;
                seed       = static_cast<result_type>(z ^ (z >> 31U));
            }
        }

        result_type operator()() {
            return GenT::next_rnd(state);
        }
    };
}  // namespace random

using Xoshiro256P  = random::XoshiroMixIn<random::Xoshiro256PImpl>;
using Xoshiro256PP = random::XoshiroMixIn<random::Xoshiro256PPImpl>;
using Xoshiro128P  = random::XoshiroMixIn<random::Xoshiro128PImpl>;
using Xoshiro128PP = random::XoshiroMixIn<random::Xoshiro128PPImpl>;

}  // namespace cft

#endif /* CFT_SRC_CORE_XOSHIRO_PRNG_HPP */