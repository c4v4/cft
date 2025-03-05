// SPDX-FileCopyrightText: 2024 Francesco Cavaliere <francescocava95@gmail.com>
// SPDX-License-Identifier: MIT

#ifndef CFT_SRC_CORE_UTILITY_HPP
#define CFT_SRC_CORE_UTILITY_HPP


#include <cstddef>
#include <utility>

#include "utils/assert.hpp"  // IWYU pragma: keep
#include "utils/custom_types.hpp"

#ifndef NDEBUG
#include "utils/limits.hpp"
#endif

namespace cft {

struct IdentityFtor {
    template <typename T>
    T&& operator()(T&& t) const {
        return std::forward<T>(t);
    }
};

struct NoOp {
    template <typename... ArgsT>
    void operator()(ArgsT&&... /*args*/) const {
    }
};

// SFINAE helpers, activate overload if T is integral or floating point
template <typename T>
using requires_integral = typename std::enable_if<std::is_integral<T>::value, bool>::type;
template <typename T>
using requires_floating_point = typename std::enable_if<std::is_floating_point<T>::value,
                                                        bool>::type;

////////////////////////////////////// CHECKED CAST //////////////////////////////////////
// We want to wrap static_cast for numeric values, checking that some properties are satisfied:
// 1. If we start from an integral type, the round-trip is lossless.
// 2. The sign is preserved between integrals casts.
// 3. The value is in range for the target type for floating point casts.
//
// NOTE: this is NOT a `narrow-cast`, information might be lost with floating points.
// NOTE: integral -> floating point must be in the range where the floating point can represent the
// integral with precision at least 1
template <typename ResT, typename T, requires_integral<native_t<T>> = true>
constexpr ResT checked_cast(T val) {
    using res_t = native_t<ResT>;
    return assert(static_cast<T>(static_cast<res_t>(val)) == val),  // Round-trip check
           assert(std::is_signed<native_t<T>>::value == std::is_signed<res_t>::value ||
                  (static_cast<res_t>(val) < res_t{}) == (val < T{})),  // Sign check
           ResT{static_cast<res_t>(val)};
}

template <typename ResT, typename T, requires_floating_point<native_t<T>> = true>
constexpr ResT checked_cast(T val) {
    // Here ResT can be integral, if val in in range, then any floating point can be casted to it
    return assert(static_cast<long double>(limits<ResT>::min()) <= static_cast<long double>(val) &&
                  static_cast<long double>(val) <= static_cast<long double>(limits<ResT>::max())),
           ResT{static_cast<native_t<ResT>>(val)};
}

///////////// COMPARISON STUFF /////////////

template <typename T, typename LT, typename UT>
T clamp(T const& v, LT const& lb, UT const& ub) {
    return (v < lb) ? static_cast<T>(lb) : (v > ub) ? static_cast<T>(ub) : v;
}

template <typename T>
T abs(T val) {
    return val < T{} ? -val : val;
}

///////////// RANGES STUFF /////////////

template <typename C>
using container_iterator_t = decltype(std::begin(std::declval<C&>()));
template <typename C>
using container_value_type_t = no_cvr<decltype(*std::declval<container_iterator_t<C>>())>;
template <typename C>
using container_size_type_t = decltype(std::declval<C>().size());

// Condition test operations
template <typename T, typename O>
bool any(T const& container, O op) {
    for (auto const& e : container)
        if (op(e))
            return true;
    return false;
}

template <typename T, typename O>
bool all(T const& container, O op) {
    for (auto const& e : container)
        if (!op(e))
            return false;
    return true;
}

// Return minimum value of a non-empty range
template <typename C, typename K = IdentityFtor>
container_value_type_t<C> range_min(C const& container, K key = {}) {
    assert(container.size() > 0ULL);
    auto min_elem = container[0];
    for (size_t i = 1ULL; i < container.size(); ++i)
        if (key(container[i]) < key(min_elem))
            min_elem = container[i];
    return min_elem;
}

// Removes elements from a container based on a given predicate, and resizes the container.
template <typename C, typename Op>
void remove_if(C& container, Op op) {
    size_t w = 0;
    for (size_t i = 0ULL; i < container.size(); ++i)
        if (!op(container[i]))
            container[w++] = container[i];
    container.resize(w);
}

}  // namespace cft


#endif /* CFT_SRC_CORE_UTILITY_HPP */
