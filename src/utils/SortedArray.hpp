// SPDX-FileCopyrightText: 2024 Francesco Cavaliere <francescocava95@gmail.com>
// SPDX-License-Identifier: MIT

#ifndef CFT_SRC_CORE_SORTED_ARRAY_HPP
#define CFT_SRC_CORE_SORTED_ARRAY_HPP

#include <cstddef>
#include <utility>

#include "utils/assert.hpp"  // IWYU pragma:  keep
#include "utils/utility.hpp"

namespace cft {
template <typename T, std::size_t Nm, typename Key = IdentityFtor>
struct SortedArray : Key /*EBO*/ {
    using iterator        = T*;
    using size_type       = size_t;
    using value_type      = T;
    using reference       = T&;
    using pointer         = T*;
    using difference_type = std::ptrdiff_t;

    T      data[Nm];
    size_t sz = 0ULL;

    // NOTE: this is a workaround, in general we cannot have EBO *and* be an aggregate.
    explicit SortedArray(Key key = {})
        : Key(key) {
    }

    size_type size() const {
        return sz;
    }

    T const& back() const {
        assert(sz > 0);
        return data[sz - 1];
    }

    T const& operator[](size_type i) const {
        assert(i < sz);
        return data[i];
    }

    T const* begin() const {
        return std::addressof(data[0]);
    }

    T const* end() const {
        return std::addressof(data[sz]);
    }

    auto key(T const& a) -> decltype(this->operator()(a)) {
        return this->operator()(a);
    }

    auto key(T const& a) const -> decltype(this->operator()(a)) {
        return this->operator()(a);
    }

    template <typename U>
    bool try_insert(U&& elem) {
        if (sz >= Nm) {
            if (key(elem) >= key(back()))
                return false;
            --sz;  // pop_back
        }
        insert(std::forward<U>(elem));
        return true;
    }

    template <typename U>
    void insert(U&& elem) {
        assert(sz < Nm);

        // find elem position
        size_type i = sz;
        while (i > 0 && key(elem) < key(data[i - 1])) {
            data[i] = std::move(data[i - 1]);
            --i;
        }
        data[i] = std::forward<U>(elem);
        ++sz;
    }

    void clear() {
        sz = 0;
    }
};

template <typename T, std::size_t Nm, typename Key>
inline SortedArray<T, Nm, no_cvr<Key>> make_custom_key_sorted_array(Key key) {
    return SortedArray<T, Nm, no_cvr<Key>>{key};
}

}  // namespace cft
#endif /*CFT_SRC_CORE_SORTED_ARRAY_HPP*/
