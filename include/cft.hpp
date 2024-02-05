#ifndef CFT_INCLUDE_CFT_HPP
#define CFT_INCLUDE_CFT_HPP

#include <cstdint>

#define NO_INLINE  //__attribute__((noinline))

#ifdef VERBOSE
#define IF_VERBOSE
#else
#define IF_VERBOSE if constexpr (false)
#endif

#ifdef NDEBUG
#define IF_DEBUG if constexpr (false)
#else
#define IF_DEBUG
#endif

using idx_t  = uint32_t;
using real_t = float;

#define REAL_MAX    (std::numeric_limits<real_t>::max())
#define REAL_LOWEST (std::numeric_limits<real_t>::lowest())

#ifdef __GNUC__
#define CFT_NODISCARD __attribute__((warn_unused_result))
#elif defined(_MSC_VER) && _MSC_VER >= 1700
#define CFT_NODISCARD _Check_return_
#endif

#include <random>

namespace cft {
using prng_t = std::mt19937_64;

template <bool C, typename T = void>
using requires_t = typename std::enable_if<C, T>::type;

}  // namespace cft

#endif /* CFT_INCLUDE_CFT_HPP */
