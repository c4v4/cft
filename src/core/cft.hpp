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

#ifndef CFT_SRC_CORE_CFT_HPP
#define CFT_SRC_CORE_CFT_HPP


#include <cstdint>
#include <string>
#include <vector>

#include "utils/Chrono.hpp"
#include "utils/assert.hpp"  // IWYU pragma:  keep
#include "utils/limits.hpp"
#include "utils/random.hpp"
#include "utils/utility.hpp"

#ifdef NDEBUG
#define CFT_IF_DEBUG(...)
#else
#define CFT_IF_DEBUG(...) __VA_ARGS__
#endif

// Noinline attribute to help profiling specific functions
#if defined(__GNUC__) || defined(__clang__)
#define CFT_NOINLINE __attribute__((noinline))
#elif defined(_MSC_VER)
#define CFT_NOINLINE __declspec(noinline)
#endif

// AVAILABLE PARSERS
#define CFT_RAIL_PARSER "RAIL"
#define CFT_SCP_PARSER  "SCP"
#define CFT_CVRP_PARSER "CVRP"
#define CFT_MPS_PARSER  "MPS"

namespace cft {

using cidx_t = int32_t;                    // Type for column indexes
using ridx_t = int16_t;                    // Type for row indexes
using real_t = float;                      // Type for real values
using prng_t = prng_picker<real_t>::type;  // default pseudo-random number generator type

// Reserved values to mark removed indexes (tombstones)
constexpr cidx_t removed_cidx = limits<cidx_t>::max();
constexpr ridx_t removed_ridx = limits<ridx_t>::max();

// Debug checked narrow cast to cidx_t
template <typename T>
constexpr cidx_t as_cidx(T val) {
    return checked_cast<cidx_t>(val);
}

// Debug checked narrow cast to ridx_t
template <typename T>
constexpr ridx_t as_ridx(T val) {
    return checked_cast<ridx_t>(val);
}

// Debug checked narrow cast to ridx_t
template <typename T>
constexpr real_t as_real(T val) {
    return checked_cast<real_t>(val);
}

// User-defined literals for cidx_t with debug and comptime checks)
constexpr cidx_t operator""_C(unsigned long long j) {
    return as_cidx(j);
}

// User-defined literals for ridx_t with debug and comptime checks)
constexpr ridx_t operator""_R(unsigned long long i) {
    return as_ridx(i);
}

// User-defined literals for real_t with debug and comptime checks)
constexpr real_t operator""_F(long double f) {
    return as_real(f);
}

// Since cidx_t could be any integer type, csize provide a (debug) checked way to get the size of a
// container as cidx_t. It also allows to avoid narrowing conversion warnings.
template <typename Cont>
inline cidx_t csize(Cont const& cont) {
    return as_cidx(cft::size(cont));
}

// Since ridx_t could be any integer type, rsize provide a (debug) checked way to get the size of a
// container as ridx_t. It also allows to avoid narrowing conversion warnings.
template <typename Cont>
inline ridx_t rsize(Cont const& cont) {
    return as_ridx(cft::size(cont));
}

struct CidxAndCost {
    cidx_t idx;
    real_t cost;
};

struct Solution {
    std::vector<cidx_t> idxs = {};
    real_t              cost = limits<real_t>::inf();
};

// Environment struct to hold all the parameters and working variables
struct Environment {
    // Cli params
    std::string inst_path        = {};                     // Instance file path
    std::string sol_path         = {};                     // Solution file path
    std::string initsol_path     = {};                     // Initial solution file path
    std::string parser           = CFT_RAIL_PARSER;        // Parser to use
    uint64_t    seed             = 0;                      // Seed for the random number generator
    double      time_limit       = limits<double>::inf();  // Time limit in seconds
    uint64_t    verbose          = 4;                      // Verbosity level
    real_t      epsilon          = 0.999999_F;  // Epsilon value for objective comparisons
    uint64_t    heur_iters       = 250;         // Number of iterations for the heuristic phase
    real_t      alpha            = 1.1_F;       // Relative fixing fraction increment
    real_t      beta             = 1.0_F;       // Relative cutoff value to terminate Refinement
    real_t      abs_subgrad_exit = 1.0_F;    // Minimum LBs delta to trigger subradient termination
    real_t      rel_subgrad_exit = 0.001_F;  // Minimum LBs gap to trigger subradient termination

    // Working params
    Chrono<>       timer = {};   // Timer to keep track of the elapsed time
    mutable prng_t rnd   = {0};  // Random number generator


    // Other hyperparameters that we might consider in the future
    // uint64_t    subgrad_exit_period      = 300;
    // double      fix_thresh               = -0.001;
    // uint64_t    stepsize_init_period     = 20;
    // double      dec_stepsize_thresh      = 0.01;
    // double      inc_stepsize_thresh      = 0.001;
    // double      dec_stepsize_factor      = 2.0;
    // double      inc_stepsize_factor      = 1.5;
    // uint64_t    init_pricing_period      = 10;
    // uint64_t    min_max_period_increment = 1000;
    // double      low_price_inc_thresh     = 1e-6;
    // uint64_t    low_price_factor         = 10;
    // double      mid_price_inc_thresh     = 0.02;
    // uint64_t    mid_price_factor         = 5;
    // double      up_price_inc_thresh      = 0.2;
    // uint64_t    up_price_factor          = 2;
    // double      c1_price_thresh          = 0.1;
    // uint64_t    c2_price_cov             = 5;
};

}  // namespace cft


#endif /* CFT_SRC_CORE_CFT_HPP */
