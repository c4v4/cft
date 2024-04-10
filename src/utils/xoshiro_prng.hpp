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

// Based on Reputeless' Xoshiro-cpp (github.com/Reputeless/Xoshiro-cpp) which is based on David
// Blackman and Sebastiano Vigna's xoshiro/xoroshiro generators (prng.di.unimi.it/)

#ifndef CFT_SRC_CORE_XOSHIRO_PRNG_HPP
#define CFT_SRC_CORE_XOSHIRO_PRNG_HPP


#include <array>
#include <cstddef>
#include <cstdint>

#include "utils/limits.hpp"

// TODO(cava): we probably need 1 max 2 of these, just test and remove the others

namespace cft {
namespace random {
    namespace {
        inline uint64_t rot_l(uint64_t x, uint32_t s) {
            return (x << s) | (x >> (64U - s));
        }

        inline uint32_t rot_l(uint32_t x, uint32_t s) {
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

    // xoshiro256+
    // Output: 64 bits
    // Period: 2^256 - 1
    // Footprint: 32 bytes
    // Original implementation: http://prng.di.unimi.it/xoshiro256plus.c
    // Version: 1.0
    class Xoshiro256PlusImpl {
    public:
        using result_type = uint64_t;
        using state_type  = std::array<result_type, 4>;
        state_type m_state;

        inline result_type operator()() {
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

        // This is the jump function for the generator. It is equivalent
        // to 2^128 calls to operator(); it can be used to generate 2^128
        // non-overlapping subsequences for parallel computations.
        inline void jump() {
            uint64_t jump_magic[] = {0x180ec6d33cfd0aba,
                                     0xd5a61266f0c9392c,
                                     0xa9582618e03fc9aa,
                                     0x39abdc4529b1661c};

            auto s = state_type{};
            for (uint64_t jump : jump_magic)
                for (uint32_t b = 0; b < 64; ++b) {
                    if ((jump & UINT64_C(1) << b) != 0) {
                        s[0] ^= m_state[0];
                        s[1] ^= m_state[1];
                        s[2] ^= m_state[2];
                        s[3] ^= m_state[3];
                    }
                    operator()();
                }
            m_state = s;
        }

        // This is the long-jump function for the generator. It is equivalent to
        // 2^192 calls to next(); it can be used to generate 2^64 starting points,
        // from each of which jump() will generate 2^64 non-overlapping
        // subsequences for parallel distributed computations.
        inline void long_jump() {
            uint64_t long_jump_magic[] = {0x76e15d3efefdcbbf,
                                          0xc5004e441c522fb3,
                                          0x77710069854ee241,
                                          0x39109bb02acbe635};

            auto s = state_type{};
            for (uint64_t jump : long_jump_magic)
                for (uint32_t b = 0; b < 64; ++b) {
                    if ((jump & UINT64_C(1) << b) != 0) {
                        s[0] ^= m_state[0];
                        s[1] ^= m_state[1];
                        s[2] ^= m_state[2];
                        s[3] ^= m_state[3];
                    }
                    operator()();
                }
            m_state = s;
        }
    };

    // xoshiro256++
    // Output: 64 bits
    // Period: 2^256 - 1
    // Footprint: 32 bytes
    // Original implementation: http://prng.di.unimi.it/xoshiro256plusplus.c
    // Version: 1.0
    class Xoshiro256PlusPlusImpl {
    public:
        using result_type = uint64_t;
        using state_type  = std::array<result_type, 4>;
        state_type m_state;

        inline result_type operator()() {
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

        // This is the jump function for the generator. It is equivalent
        // to 2^128 calls to next(); it can be used to generate 2^128
        // non-overlapping subsequences for parallel computations.
        inline void jump() {
            uint64_t jump_magic[] = {0x180ec6d33cfd0aba,
                                     0xd5a61266f0c9392c,
                                     0xa9582618e03fc9aa,
                                     0x39abdc4529b1661c};

            auto s = state_type{};
            for (uint64_t jump : jump_magic)
                for (uint32_t b = 0; b < 64; ++b) {
                    if ((jump & UINT64_C(1) << b) != 0) {
                        s[0] ^= m_state[0];
                        s[1] ^= m_state[1];
                        s[2] ^= m_state[2];
                        s[3] ^= m_state[3];
                    }
                    operator()();
                }
            m_state = s;
        }

        // This is the long-jump function for the generator. It is equivalent to
        // 2^192 calls to next(); it can be used to generate 2^64 starting points,
        // from each of which jump() will generate 2^64 non-overlapping
        // subsequences for parallel distributed computations.
        inline void long_jump() {
            uint64_t long_jump_magic[] = {0x76e15d3efefdcbbf,
                                          0xc5004e441c522fb3,
                                          0x77710069854ee241,
                                          0x39109bb02acbe635};

            auto s = state_type{};
            for (uint64_t jump : long_jump_magic)
                for (uint32_t b = 0; b < 64; ++b) {
                    if ((jump & UINT64_C(1) << b) != 0) {
                        s[0] ^= m_state[0];
                        s[1] ^= m_state[1];
                        s[2] ^= m_state[2];
                        s[3] ^= m_state[3];
                    }
                    operator()();
                }
            m_state = s;
        }
    };

    // xoshiro256**
    // Output: 64 bits
    // Period: 2^256 - 1
    // Footprint: 32 bytes
    // Original implementation: http://prng.di.unimi.it/xoshiro256starstar.c
    // Version: 1.0
    class Xoshiro256StarStarImpl {
    public:
        using result_type = uint64_t;
        using state_type  = std::array<result_type, 4>;
        state_type m_state;

        inline result_type operator()() {
            uint64_t result = rot_l(m_state[1] * 5, 7) * 9;
            uint64_t t      = m_state[1] << 17U;
            m_state[2] ^= m_state[0];
            m_state[3] ^= m_state[1];
            m_state[1] ^= m_state[2];
            m_state[0] ^= m_state[3];
            m_state[2] ^= t;
            m_state[3] = rot_l(m_state[3], 45);
            return result;
        }

        // This is the jump function for the generator. It is equivalent
        // to 2^128 calls to next(); it can be used to generate 2^128
        // non-overlapping subsequences for parallel computations.
        inline void jump() {
            uint64_t jump_magic[] = {0x180ec6d33cfd0aba,
                                     0xd5a61266f0c9392c,
                                     0xa9582618e03fc9aa,
                                     0x39abdc4529b1661c};

            auto s = state_type{};
            for (uint64_t jump : jump_magic)
                for (uint32_t b = 0; b < 64; ++b) {
                    if ((jump & UINT64_C(1) << b) != 0) {
                        s[0] ^= m_state[0];
                        s[1] ^= m_state[1];
                        s[2] ^= m_state[2];
                        s[3] ^= m_state[3];
                    }
                    operator()();
                }
            m_state = s;
        }

        // This is the long-jump function for the generator. It is equivalent to
        // 2^192 calls to next(); it can be used to generate 2^64 starting points,
        // from each of which jump() will generate 2^64 non-overlapping
        // subsequences for parallel distributed computations.
        inline void long_jump() {
            uint64_t long_jump_magic[] = {0x76e15d3efefdcbbf,
                                          0xc5004e441c522fb3,
                                          0x77710069854ee241,
                                          0x39109bb02acbe635};

            auto s = state_type{};
            for (uint64_t jump : long_jump_magic)
                for (uint32_t b = 0; b < 64; ++b) {
                    if ((jump & UINT64_C(1) << b) != 0) {
                        s[0] ^= m_state[0];
                        s[1] ^= m_state[1];
                        s[2] ^= m_state[2];
                        s[3] ^= m_state[3];
                    }
                    operator()();
                }
            m_state = s;
        }
    };

    // xoroshiro128+
    // Output: 64 bits
    // Period: 2^128 - 1
    // Footprint: 16 bytes
    // Original implementation: http://prng.di.unimi.it/xoroshiro128plus.c
    // Version: 1.0
    class Xoroshiro128Plus {
    public:
        using result_type = uint64_t;
        using state_type  = std::array<result_type, 2>;
        state_type m_state;

        inline Xoroshiro128Plus(uint64_t seed = default_seed)
            : m_state(generate_state_from_seed<uint64_t, 2>(seed)) {
        }

        inline Xoroshiro128Plus(state_type const state)
            : m_state(state) {
        }

        inline result_type operator()() {
            uint64_t s0     = m_state[0];
            uint64_t s1     = m_state[1];
            uint64_t result = s0 + s1;
            s1 ^= s0;
            m_state[0] = rot_l(s0, 24) ^ s1 ^ (s1 << 16U);
            m_state[1] = rot_l(s1, 37);
            return result;
        }

        // This is the jump function for the generator. It is equivalent
        // to 2^64 calls to next(); it can be used to generate 2^64
        // non-overlapping subsequences for parallel computations.
        inline void jump() {
            uint64_t jump_magic[] = {0xdf900294d8f554a5, 0x170865df4b3201fc};

            auto s = state_type{};
            for (uint64_t jump : jump_magic)
                for (uint32_t b = 0; b < 64; ++b) {
                    if ((jump & UINT64_C(1) << b) != 0) {
                        s[0] ^= m_state[0];
                        s[1] ^= m_state[1];
                    }
                    operator()();
                }
            m_state = s;
        }

        // This is the long-jump function for the generator. It is equivalent to
        // 2^96 calls to next(); it can be used to generate 2^32 starting points,
        // from each of which jump() will generate 2^32 non-overlapping
        // subsequences for parallel distributed computations.
        inline void long_jump() {
            uint64_t long_jump_magic[] = {0xd2a98b26625eee7b, 0xdddf9b1090aa7ac1};

            auto s = state_type{};
            for (uint64_t jump : long_jump_magic)
                for (uint32_t b = 0; b < 64; ++b) {
                    if ((jump & UINT64_C(1) << b) != 0) {
                        s[0] ^= m_state[0];
                        s[1] ^= m_state[1];
                    }
                    operator()();
                }
            m_state = s;
        }
    };

    // xoroshiro128++
    // Output: 64 bits
    // Period: 2^128 - 1
    // Footprint: 16 bytes
    // Original implementation: http://prng.di.unimi.it/xoroshiro128plusplus.c
    // Version: 1.0
    class Xoroshiro128PlusPlus {
    public:
        using result_type = uint64_t;
        using state_type  = std::array<result_type, 2>;
        state_type m_state;

        inline Xoroshiro128PlusPlus(uint64_t seed = default_seed)
            : m_state(generate_state_from_seed<uint64_t, 2>(seed)) {
        }

        inline Xoroshiro128PlusPlus(state_type const state)
            : m_state(state) {
        }

        inline result_type operator()() {
            uint64_t s0     = m_state[0];
            uint64_t s1     = m_state[1];
            uint64_t result = rot_l(s0 + s1, 17) + s0;
            s1 ^= s0;
            m_state[0] = rot_l(s0, 49) ^ s1 ^ (s1 << 21U);
            m_state[1] = rot_l(s1, 28);
            return result;
        }

        // This is the jump function for the generator. It is equivalent
        // to 2^64 calls to next(); it can be used to generate 2^64
        // non-overlapping subsequences for parallel computations.
        inline void jump() {
            uint64_t jump_magic[] = {0x2bd7a6a6e99c2ddc, 0x0992ccaf6a6fca05};

            auto s = state_type{};
            for (uint64_t jump : jump_magic)
                for (uint32_t b = 0; b < 64; ++b) {
                    if ((jump & UINT64_C(1) << b) != 0) {
                        s[0] ^= m_state[0];
                        s[1] ^= m_state[1];
                    }
                    operator()();
                }
            m_state = s;
        }

        // This is the long-jump function for the generator. It is equivalent to
        // 2^96 calls to next(); it can be used to generate 2^32 starting points,
        // from each of which jump() will generate 2^32 non-overlapping
        // subsequences for parallel distributed computations.
        inline void long_jump() {
            uint64_t long_jump_magic[] = {0x360fd5f2cf8d5d99, 0x9c6e6877736c46e3};

            auto s = state_type{};
            for (uint64_t jump : long_jump_magic)
                for (uint32_t b = 0; b < 64; ++b) {
                    if ((jump & UINT64_C(1) << b) != 0) {
                        s[0] ^= m_state[0];
                        s[1] ^= m_state[1];
                    }
                    operator()();
                }
            m_state = s;
        }
    };

    // xoroshiro128**
    // Output: 64 bits
    // Period: 2^128 - 1
    // Footprint: 16 bytes
    // Original implementation: http://prng.di.unimi.it/xoroshiro128starstar.c
    // Version: 1.0
    class Xoroshiro128StarStar {
    public:
        using result_type = uint64_t;
        using state_type  = std::array<result_type, 2>;
        state_type m_state;

        inline Xoroshiro128StarStar(uint64_t seed = default_seed)
            : m_state(generate_state_from_seed<uint64_t, 2>(seed)) {
        }

        inline Xoroshiro128StarStar(state_type const state)
            : m_state(state) {
        }

        inline result_type operator()() {
            uint64_t s0     = m_state[0];
            uint64_t s1     = m_state[1];
            uint64_t result = rot_l(s0 * 5, 7) * 9;
            s1 ^= s0;
            m_state[0] = rot_l(s0, 24) ^ s1 ^ (s1 << 16U);
            m_state[1] = rot_l(s1, 37);
            return result;
        }

        // This is the jump function for the generator. It is equivalent
        // to 2^64 calls to next(); it can be used to generate 2^64
        // non-overlapping subsequences for parallel computations.
        inline void jump() {
            uint64_t jump_magic[] = {0xdf900294d8f554a5, 0x170865df4b3201fc};

            auto s = state_type{};
            for (uint64_t jump : jump_magic)
                for (uint32_t b = 0; b < 64; ++b) {
                    if ((jump & UINT64_C(1) << b) != 0) {
                        s[0] ^= m_state[0];
                        s[1] ^= m_state[1];
                    }
                    operator()();
                }
            m_state = s;
        }

        // This is the long-jump function for the generator. It is equivalent to
        // 2^96 calls to next(); it can be used to generate 2^32 starting points,
        // from each of which jump() will generate 2^32 non-overlapping
        // subsequences for parallel distributed computations.
        inline void long_jump() {
            uint64_t long_jump_magic[] = {0xd2a98b26625eee7b, 0xdddf9b1090aa7ac1};

            auto s = state_type{};
            for (uint64_t jump : long_jump_magic)
                for (uint32_t b = 0; b < 64; ++b) {
                    if ((jump & UINT64_C(1) << b) != 0) {
                        s[0] ^= m_state[0];
                        s[1] ^= m_state[1];
                    }
                    operator()();
                }
            m_state = s;
        }
    };

    // xoshiro128+
    // Output: 32 bits
    // Period: 2^128 - 1
    // Footprint: 16 bytes
    // Original implementation: http://prng.di.unimi.it/xoshiro128plus.c
    // Version: 1.0
    class Xoshiro128PlusImpl {
    public:
        using result_type = uint32_t;
        using state_type  = std::array<result_type, 4>;
        state_type m_state;

        inline result_type operator()() {
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

        // This is the jump function for the generator. It is equivalent
        // to 2^64 calls to next(); it can be used to generate 2^64
        // non-overlapping subsequences for parallel computations.
        inline void jump() {
            uint32_t jump_magic[] = {0x8764000b, 0xf542d2d3, 0x6fa035c3, 0x77f2db5b};

            auto s = state_type{};
            for (uint32_t jump : jump_magic)
                for (uint32_t b = 0; b < 32; ++b) {
                    if ((jump & UINT32_C(1) << b) != 0) {
                        s[0] ^= m_state[0];
                        s[1] ^= m_state[1];
                        s[2] ^= m_state[2];
                        s[3] ^= m_state[3];
                    }
                    operator()();
                }
            m_state = s;
        }

        // This is the long-jump function for the generator. It is equivalent to
        // 2^96 calls to next(); it can be used to generate 2^32 starting points,
        // from each of which jump() will generate 2^32 non-overlapping
        // subsequences for parallel distributed computations.
        inline void long_jump() {
            uint32_t long_jump_magic[] = {0xb523952e, 0x0b6f099f, 0xccf5a0ef, 0x1c580662};

            auto s = state_type{};
            for (uint32_t jump : long_jump_magic)
                for (uint32_t b = 0; b < 32; ++b) {
                    if ((jump & UINT32_C(1) << b) != 0) {
                        s[0] ^= m_state[0];
                        s[1] ^= m_state[1];
                        s[2] ^= m_state[2];
                        s[3] ^= m_state[3];
                    }
                    operator()();
                }
            m_state = s;
        }
    };

    // xoshiro128++
    // Output: 32 bits
    // Period: 2^128 - 1
    // Footprint: 16 bytes
    // Original implementation: http://prng.di.unimi.it/xoshiro128plusplus.c
    // Version: 1.0
    class Xoshiro128PlusPlusImpl {
    public:
        using result_type = uint32_t;
        using state_type  = std::array<result_type, 4>;
        state_type m_state;

        inline result_type operator()() {
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

        // This is the jump function for the generator. It is equivalent
        // to 2^64 calls to next(); it can be used to generate 2^64
        // non-overlapping subsequences for parallel computations.
        inline void jump() {
            uint32_t jump_magic[] = {0x8764000b, 0xf542d2d3, 0x6fa035c3, 0x77f2db5b};

            auto s = state_type{};
            for (uint32_t jump : jump_magic)
                for (uint32_t b = 0; b < 32; ++b) {
                    if ((jump & UINT32_C(1) << b) != 0) {
                        s[0] ^= m_state[0];
                        s[1] ^= m_state[1];
                        s[2] ^= m_state[2];
                        s[3] ^= m_state[3];
                    }
                    operator()();
                }
            m_state = s;
        }

        // This is the long-jump function for the generator. It is equivalent to
        // 2^96 calls to next(); it can be used to generate 2^32 starting points,
        // from each of which jump() will generate 2^32 non-overlapping
        // subsequences for parallel distributed computations.
        inline void long_jump() {
            uint32_t long_jump_magic[] = {0xb523952e, 0x0b6f099f, 0xccf5a0ef, 0x1c580662};

            auto s = state_type{};
            for (uint32_t jump : long_jump_magic)
                for (uint32_t b = 0; b < 32; ++b) {
                    if ((jump & UINT32_C(1) << b) != 0) {
                        s[0] ^= m_state[0];
                        s[1] ^= m_state[1];
                        s[2] ^= m_state[2];
                        s[3] ^= m_state[3];
                    }
                    operator()();
                }
            m_state = s;
        }
    };

    // xoshiro128**
    // Output: 32 bits
    // Period: 2^128 - 1
    // Footprint: 16 bytes
    // Original implementation: http://prng.di.unimi.it/xoshiro128starstar.c
    // Version: 1.1
    class Xoshiro128StarStarImpl {
    public:
        using result_type = uint32_t;
        using state_type  = std::array<result_type, 4>;
        state_type m_state;

        inline result_type operator()() {
            uint32_t result = rot_l(m_state[1] * 5, 7) * 9;
            uint32_t t      = m_state[1] << 9U;
            m_state[2] ^= m_state[0];
            m_state[3] ^= m_state[1];
            m_state[1] ^= m_state[2];
            m_state[0] ^= m_state[3];
            m_state[2] ^= t;
            m_state[3] = rot_l(m_state[3], 11);
            return result;
        }

        // This is the jump function for the generator. It is equivalent
        // to 2^64 calls to next(); it can be used to generate 2^64
        // non-overlapping subsequences for parallel computations.
        inline void jump() {
            uint32_t jump_magic[] = {0x8764000b, 0xf542d2d3, 0x6fa035c3, 0x77f2db5b};

            auto s = state_type{};
            for (uint32_t jump : jump_magic)
                for (uint32_t b = 0; b < 32; ++b) {
                    if ((jump & UINT32_C(1) << b) != 0) {
                        s[0] ^= m_state[0];
                        s[1] ^= m_state[1];
                        s[2] ^= m_state[2];
                        s[3] ^= m_state[3];
                    }
                    operator()();
                }
            m_state = s;
        }

        // This is the long-jump function for the generator. It is equivalent to
        // 2^96 calls to next(); it can be used to generate 2^32 starting points,
        // from each of which jump() will generate 2^32 non-overlapping
        // subsequences for parallel distributed computations.
        inline void long_jump() {
            uint32_t long_jump_magic[] = {0xb523952e, 0x0b6f099f, 0xccf5a0ef, 0x1c580662};

            auto s = state_type{};
            for (uint32_t jump : long_jump_magic)
                for (uint32_t b = 0; b < 32; ++b) {
                    if ((jump & UINT32_C(1) << b) != 0) {
                        s[0] ^= m_state[0];
                        s[1] ^= m_state[1];
                        s[2] ^= m_state[2];
                        s[3] ^= m_state[3];
                    }
                    operator()();
                }
            m_state = s;
        }
    };

    template <typename GenT>
    struct RandMixIn : GenT {
        using result_type = typename GenT::result_type;
        using state_type  = typename GenT::state_type;

        RandMixIn(uint64_t seed = default_seed)
            : GenT{generate_state_from_seed<result_type, 4>(seed)} {
        }

        RandMixIn(state_type state)
            : GenT{state} {
        }

        static constexpr result_type min() {
            return limits<result_type>::min();
        }

        static constexpr result_type max() {
            return limits<result_type>::max();
        }

        state_type serialize() const {
            return GenT::m_state;
        }

        void deserialize(state_type const state) {
            GenT::m_state = state;
        }

        bool operator==(GenT rhs) const {
            return GenT::m_state == rhs.m_state;
        }

        bool operator!=(GenT rhs) const {
            return GenT::m_state != rhs.m_state;
        }
    };
}  // namespace random

using Xoshiro256Plus     = random::RandMixIn<random::Xoshiro256PlusImpl>;
using Xoshiro256PlusPlus = random::RandMixIn<random::Xoshiro256PlusPlusImpl>;
using Xoshiro256StarStar = random::RandMixIn<random::Xoshiro256StarStarImpl>;
using Xoshiro128Plus     = random::RandMixIn<random::Xoshiro128PlusImpl>;
using Xoshiro128PlusPlus = random::RandMixIn<random::Xoshiro128PlusPlusImpl>;
using Xoshiro128StarStar = random::RandMixIn<random::Xoshiro128StarStarImpl>;

}  // namespace cft


#endif /* CFT_SRC_CORE_XOSHIRO_PRNG_HPP */
