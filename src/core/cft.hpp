#ifndef CFT_SRC_CORE_CFT_HPP
#define CFT_SRC_CORE_CFT_HPP

#include <cstdint>
#include <vector>

#include "core/limits.hpp"

#ifdef VERBOSE
#define IF_VERBOSE(...) __VA_ARGS__
#else
#define IF_VERBOSE(...)
#endif

#ifdef NDEBUG
#define IF_DEBUG(...)
#else
#define IF_DEBUG(...) __VA_ARGS__
#endif

// Reserve column index to mark removed/invalid columns
#define CFT_REMOVED_IDX (cft::limits<cidx_t>::max())

// Epsilon value for floating point comparisons of costs
#define CFT_EPSILON cft::real_t(1 - 1e-6)  // 1-1e-6 for integer costs, 1e-6 for float costs

namespace cft {

using cidx_t = uint32_t;
using ridx_t = uint32_t;
using real_t = float;

struct CidxAndCost {
    cidx_t col;
    real_t cost;
};

struct Solution {
    std::vector<cft::cidx_t> idxs = {};
    cft::real_t              cost = cft::limits<cft::real_t>::max();
};

}  // namespace cft

// Nodiscard attribute to warn if the return value is not used
#ifdef __GNUC__
#define CFT_NODISCARD __attribute__((warn_unused_result))
#elif defined(_MSC_VER) && _MSC_VER >= 1700
#define CFT_NODISCARD _Check_return_
#endif

// Noinline attribute to help profiling specific functions
#if defined(__GNUC__) || defined(__clang__)
#define CFT_NOINLINE __attribute__((noinline))
#elif defined(_MSC_VER)
#define CFT_NOINLINE __declspec(noinline)
#endif


#endif /* CFT_SRC_CORE_CFT_HPP */
