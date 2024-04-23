// SPDX-FileCopyrightText: 2024 Francesco Cavaliere <francescocava95@gmail.com>
// SPDX-License-Identifier: MIT

#ifndef CFT_INCLUDE_UTILS_TAGGEDSCALAR_HPP
#define CFT_INCLUDE_UTILS_TAGGEDSCALAR_HPP

#include <cassert>

#ifndef NDEBUG
#include <cmath>
#endif

namespace ext {
// TaggedScalar wraps a native arithmetic type and define almost all conversion as explicit. The
// only implicit conversion allowed is from TaggedScalar<T> to T.
// It can be considered as a "soft" type-safe wrapper.
template <typename T, typename TAG>
struct TaggedScalar {
    using type = T;
    T value;

    constexpr TaggedScalar() = default;

    constexpr explicit TaggedScalar(T val)
        : value(val) {
    }

    // This must be implicit for the abstraction to work
    constexpr operator T() const {
        return assert(std::isfinite(value)), value;
    }
};

template <typename T, typename TagT>
constexpr TaggedScalar<T, TagT> operator+(TaggedScalar<T, TagT> n) {
    return assert(std::isfinite(n.value)), n;
}

template <typename T, typename TagT>
constexpr TaggedScalar<T, TagT> operator-(TaggedScalar<T, TagT> n) {
    return assert(std::isfinite(-n.value)), TaggedScalar<T, TagT>(-n.value);
}

template <typename T, typename TagT>
constexpr TaggedScalar<T, TagT>& operator++(TaggedScalar<T, TagT>& n) {
    return ++n.value, assert(std::isfinite(n.value)), n;
}

template <typename T, typename TagT>
constexpr TaggedScalar<T, TagT>& operator--(TaggedScalar<T, TagT>& n) {
    return --n.value, assert(std::isfinite(n.value)), n;
}

template <typename T, typename TagT>
TaggedScalar<T, TagT> operator++(TaggedScalar<T, TagT>& n, int) {
    auto old = n;
    ++n.value;
    assert(std::isfinite(n.value));
    return old;
}

template <typename T, typename TagT>
TaggedScalar<T, TagT> operator--(TaggedScalar<T, TagT>& n, int) {
    auto old = n;
    --n.value;
    assert(std::isfinite(n.value));
    return old;
}

template <typename T, typename TagT>
constexpr TaggedScalar<T, TagT> operator+(TaggedScalar<T, TagT> n, TaggedScalar<T, TagT> other) {
    return TaggedScalar<T, TagT>(n.value + other.value);
}

template <typename T, typename TagT>
constexpr TaggedScalar<T, TagT> operator-(TaggedScalar<T, TagT> n, TaggedScalar<T, TagT> other) {
    return TaggedScalar<T, TagT>(n.value - other.value);
}

template <typename T, typename TagT>
constexpr TaggedScalar<T, TagT> operator*(TaggedScalar<T, TagT> n, TaggedScalar<T, TagT> other) {
    return TaggedScalar<T, TagT>(n.value * other.value);
}

template <typename T, typename TagT>
constexpr TaggedScalar<T, TagT> operator/(TaggedScalar<T, TagT> n, TaggedScalar<T, TagT> other) {
    return TaggedScalar<T, TagT>(n.value / other.value);
}

template <typename T, typename TagT>
constexpr TaggedScalar<T, TagT>& operator+=(TaggedScalar<T, TagT>& n, TaggedScalar<T, TagT> other) {
    return n.value += other.value, n;
}

template <typename T, typename TagT>
constexpr TaggedScalar<T, TagT>& operator-=(TaggedScalar<T, TagT>& n, TaggedScalar<T, TagT> other) {
    return n.value -= other.value, n;
}

template <typename T, typename TagT>
constexpr TaggedScalar<T, TagT>& operator*=(TaggedScalar<T, TagT>& n, TaggedScalar<T, TagT> other) {
    return n.value *= other.value, n;
}

template <typename T, typename TagT>
constexpr TaggedScalar<T, TagT>& operator/=(TaggedScalar<T, TagT>& n, TaggedScalar<T, TagT> other) {
    return n.value /= other.value, n;
}

template <typename T, typename TagT>
constexpr bool operator<(TaggedScalar<T, TagT> n, TaggedScalar<T, TagT> other) {
    return n.value < other.value;
}

template <typename T, typename TagT>
constexpr bool operator>(TaggedScalar<T, TagT> n, TaggedScalar<T, TagT> other) {
    return n.value > other.value;
}

template <typename T, typename TagT>
constexpr bool operator<=(TaggedScalar<T, TagT> n, TaggedScalar<T, TagT> other) {
    return n.value <= other.value;
}

template <typename T, typename TagT>
constexpr bool operator>=(TaggedScalar<T, TagT> n, TaggedScalar<T, TagT> other) {
    return n.value >= other.value;
}

template <typename T, typename TagT>
constexpr bool operator==(TaggedScalar<T, TagT> n, TaggedScalar<T, TagT> other) {
    return n.value == other.value;
}

template <typename T, typename TagT>
constexpr bool operator!=(TaggedScalar<T, TagT> n, TaggedScalar<T, TagT> other) {
    return n.value != other.value;
}

// Delete any operation between a TaggedScalar and any other type
template <typename T1, typename Tag1>
bool operator!(TaggedScalar<T1, Tag1>) = delete;

template <typename T1, typename Tag1, typename T2>
TaggedScalar<T1, Tag1> operator+(TaggedScalar<T1, Tag1>, T2) = delete;
template <typename T1, typename Tag1, typename T2>
TaggedScalar<T1, Tag1> operator+(T2, TaggedScalar<T1, Tag1>) = delete;
template <typename T1, typename Tag1, typename T2>
TaggedScalar<T1, Tag1> operator-(TaggedScalar<T1, Tag1>, T2) = delete;
template <typename T1, typename Tag1, typename T2>
TaggedScalar<T1, Tag1> operator-(T2, TaggedScalar<T1, Tag1>) = delete;
template <typename T1, typename Tag1, typename T2>
TaggedScalar<T1, Tag1> operator*(TaggedScalar<T1, Tag1>, T2) = delete;
template <typename T1, typename Tag1, typename T2>
TaggedScalar<T1, Tag1> operator*(T2, TaggedScalar<T1, Tag1>) = delete;
template <typename T1, typename Tag1, typename T2>
TaggedScalar<T1, Tag1> operator/(TaggedScalar<T1, Tag1>, T2) = delete;
template <typename T1, typename Tag1, typename T2>
TaggedScalar<T1, Tag1> operator/(T2, TaggedScalar<T1, Tag1>) = delete;
template <typename T1, typename Tag1, typename T2>
TaggedScalar<T1, Tag1>& operator+=(TaggedScalar<T1, Tag1>&, T2) = delete;
template <typename T1, typename Tag1, typename T2>
TaggedScalar<T1, Tag1>& operator+=(T2&, TaggedScalar<T1, Tag1>) = delete;
template <typename T1, typename Tag1, typename T2>
TaggedScalar<T1, Tag1>& operator-=(TaggedScalar<T1, Tag1>&, T2) = delete;
template <typename T1, typename Tag1, typename T2>
TaggedScalar<T1, Tag1>& operator-=(T2&, TaggedScalar<T1, Tag1>) = delete;
template <typename T1, typename Tag1, typename T2>
TaggedScalar<T1, Tag1>& operator*=(TaggedScalar<T1, Tag1>&, T2) = delete;
template <typename T1, typename Tag1, typename T2>
TaggedScalar<T1, Tag1>& operator*=(T2&, TaggedScalar<T1, Tag1>) = delete;
template <typename T1, typename Tag1, typename T2>
TaggedScalar<T1, Tag1>& operator/=(TaggedScalar<T1, Tag1>&, T2) = delete;
template <typename T1, typename Tag1, typename T2>
TaggedScalar<T1, Tag1>& operator/=(T2&, TaggedScalar<T1, Tag1>) = delete;

template <typename T1, typename Tag1, typename T2>
bool operator<(TaggedScalar<T1, Tag1>, T2) = delete;
template <typename T1, typename Tag1, typename T2>
bool operator<(T2, TaggedScalar<T1, Tag1>) = delete;
template <typename T1, typename Tag1, typename T2>
bool operator>(TaggedScalar<T1, Tag1>, T2) = delete;
template <typename T1, typename Tag1, typename T2>
bool operator>(T2, TaggedScalar<T1, Tag1>) = delete;
template <typename T1, typename Tag1, typename T2>
bool operator<=(TaggedScalar<T1, Tag1>, T2) = delete;
template <typename T1, typename Tag1, typename T2>
bool operator<=(T2, TaggedScalar<T1, Tag1>) = delete;
template <typename T1, typename Tag1, typename T2>
bool operator>=(TaggedScalar<T1, Tag1>, T2) = delete;
template <typename T1, typename Tag1, typename T2>
bool operator>=(T2, TaggedScalar<T1, Tag1>) = delete;
template <typename T1, typename Tag1, typename T2>
bool operator==(TaggedScalar<T1, Tag1>, T2) = delete;
template <typename T1, typename Tag1, typename T2>
bool operator==(T2, TaggedScalar<T1, Tag1>) = delete;
template <typename T1, typename Tag1, typename T2>
bool operator!=(TaggedScalar<T1, Tag1>, T2) = delete;
template <typename T1, typename Tag1, typename T2>
bool operator!=(T2, TaggedScalar<T1, Tag1>) = delete;

template <typename T1, typename Tag1, typename T2>
bool operator>>(TaggedScalar<T1, Tag1>, T2) = delete;
template <typename T1, typename Tag1, typename T2>
bool operator>>(T2, TaggedScalar<T1, Tag1>) = delete;
template <typename T1, typename Tag1, typename T2>
bool operator<<(TaggedScalar<T1, Tag1>, T2) = delete;
template <typename T1, typename Tag1, typename T2>
bool operator<<(T2, TaggedScalar<T1, Tag1>) = delete;
template <typename T1, typename Tag1, typename T2>
bool operator^(TaggedScalar<T1, Tag1>, T2) = delete;
template <typename T1, typename Tag1, typename T2>
bool operator^(T2, TaggedScalar<T1, Tag1>) = delete;
template <typename T1, typename Tag1, typename T2>
bool operator|(TaggedScalar<T1, Tag1>, T2) = delete;
template <typename T1, typename Tag1, typename T2>
bool operator|(T2, TaggedScalar<T1, Tag1>) = delete;
template <typename T1, typename Tag1, typename T2>
bool operator&(TaggedScalar<T1, Tag1>, T2) = delete;
template <typename T1, typename Tag1, typename T2>
bool operator&(T2, TaggedScalar<T1, Tag1>) = delete;

}  // namespace ext


#endif /* CFT_INCLUDE_UTILS_TAGGEDSCALAR_HPP */
