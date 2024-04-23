// SPDX-FileCopyrightText: 2024 Francesco Cavaliere <francescocava95@gmail.com>
// SPDX-License-Identifier: MIT

#ifndef CFT_SRC_UTILS_MP_HPP
#define CFT_SRC_UTILS_MP_HPP

#include <type_traits>
#include <utility>

namespace cft {
// Remove const, volatile and reference from a type
template <typename T>
using no_cvr = typename std::remove_reference<typename std::remove_cv<T>::type>::type;

// Hook for user defined types, specialize this to get the base type.
template <typename T>
struct native {
    using type = no_cvr<T>;
};

template <typename T>
using native_t = typename native<no_cvr<T>>::type;

template <typename T>
constexpr native_t<T> native_cast(T&& t) {
    return std::forward<T>(t);
}


}  // namespace cft


#endif /* CFT_SRC_UTILS_MP_HPP */
