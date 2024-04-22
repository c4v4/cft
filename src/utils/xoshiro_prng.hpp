// SPDX-FileCopyrightText: 2024 Francesco Cavaliere <francescocava95@gmail.com>
// SPDX-License-Identifier: MIT

// Based on Reputeless' Xoshiro-cpp (github.com/Reputeless/Xoshiro-cpp) which is based on David
// Blackman and Sebastiano Vigna's xoshiro/xoroshiro generators (prng.di.unimi.it/)#ifndef

#ifndef CFT_SRC_CORE_XOSHIRO_PRNG_HPP
#define CFT_SRC_CORE_XOSHIRO_PRNG_HPP


#include <array>
#include <cstddef>
#include <cstdint>

#include "utils/limits.hpp"

namespace cft {
namespace random {
    namespace {
        constexpr uint64_t rot_l(uint64_t x, uint32_t s) {
            return (x << s) | (x >> (64U - s));
        }

        constexpr uint32_t rot_l(uint32_t x, uint32_t s) {
            return (x << s) | (x >> (32U - s));
        }
    }  // namespace

    constexpr uint64_t default_seed = 0x9e3779b97f4a7c15ULL;  // Default seed value

    // SplitMix64
    template <typename IntT, size_t N>
    std::array<IntT, N> generate_state_from_seed(uint64_t init_seed = default_seed) {
        auto     seeds   = std::array<IntT, N>{};
        uint64_t m_state = init_seed;
        for (auto& seed : seeds) {
            uint64_t z = (m_state += 0x9e3779b97f4a7c15);
            z          = (z ^ (z >> 30U)) * 0xbf58476d1ce4e5b9;
            z          = (z ^ (z >> 27U)) * 0x94d049bb133111eb;
            seed       = static_cast<IntT>(z ^ (z >> 31U));
        }
        return seeds;
    }

    // Source: http://prng.di.unimi.it/xoshiro256plus.c
    class Xoshiro256PlusImpl {
    public:
        using result_type = uint64_t;
        using state_type  = std::array<result_type, 4>;
        state_type m_state;

        result_type operator()() {
            uint64_t result = m_state[0] + m_state[3];
            uint64_t t      = m_state[1] << 17U;
            m_state[2] ^= m_state[0];
            m_state[3] ^= m_state[1];
            m_state[1] ^= m_state[2];
            m_state[0] ^= m_state[3];
            m_state[2] ^= t;
            m_state[3] = rot_l(m_state[3], 45);
            return result;
        }
    };

    // Source: http://prng.di.unimi.it/xoshiro256plusplus.c
    class Xoshiro256PlusPlusImpl {
    public:
        using result_type = uint64_t;
        using state_type  = std::array<result_type, 4>;
        state_type m_state;

        result_type operator()() {
            uint64_t result = rot_l(m_state[0] + m_state[3], 23) + m_state[0];
            uint64_t t      = m_state[1] << 17U;
            m_state[2] ^= m_state[0];
            m_state[3] ^= m_state[1];
            m_state[1] ^= m_state[2];
            m_state[0] ^= m_state[3];
            m_state[2] ^= t;
            m_state[3] = rot_l(m_state[3], 45);
            return result;
        }
    };

    // Source: http://prng.di.unimi.it/xoshiro128plus.c
    class Xoshiro128PlusImpl {
    public:
        using result_type = uint32_t;
        using state_type  = std::array<result_type, 4>;
        state_type m_state;

        result_type operator()() {
            uint32_t result = m_state[0] + m_state[3];
            uint32_t t      = m_state[1] << 9U;
            m_state[2] ^= m_state[0];
            m_state[3] ^= m_state[1];
            m_state[1] ^= m_state[2];
            m_state[0] ^= m_state[3];
            m_state[2] ^= t;
            m_state[3] = rot_l(m_state[3], 11);
            return result;
        }
    };

    // Source: http://prng.di.unimi.it/xoshiro128plusplus.c
    class Xoshiro128PlusPlusImpl {
    public:
        using result_type = uint32_t;
        using state_type  = std::array<result_type, 4>;
        state_type m_state;

        result_type operator()() {
            uint32_t result = rot_l(m_state[0] + m_state[3], 7) + m_state[0];
            uint32_t t      = m_state[1] << 9U;
            m_state[2] ^= m_state[0];
            m_state[3] ^= m_state[1];
            m_state[1] ^= m_state[2];
            m_state[0] ^= m_state[3];
            m_state[2] ^= t;
            m_state[3] = rot_l(m_state[3], 11);
            return result;
        }
    };

    template <typename GenT>
    struct RandMixIn : GenT {
        using result_type = typename GenT::result_type;
        using state_type  = typename GenT::state_type;

        explicit RandMixIn(uint64_t seed = default_seed)
            : GenT{generate_state_from_seed<result_type, 4>(seed)} {
        }

        static constexpr result_type min() {
            return limits<result_type>::min();
        }

        static constexpr result_type max() {
            return limits<result_type>::max();
        }
    };
}  // namespace random

using Xoshiro256Plus     = random::RandMixIn<random::Xoshiro256PlusImpl>;
using Xoshiro256PlusPlus = random::RandMixIn<random::Xoshiro256PlusPlusImpl>;
using Xoshiro128Plus     = random::RandMixIn<random::Xoshiro128PlusImpl>;
using Xoshiro128PlusPlus = random::RandMixIn<random::Xoshiro128PlusPlusImpl>;

}  // namespace cft


#endif /* CFT_SRC_CORE_XOSHIRO_PRNG_HPP */
