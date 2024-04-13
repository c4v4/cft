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


#ifndef CFT_SRC_CORE_SORTED_ARRAY_HPP
#define CFT_SRC_CORE_SORTED_ARRAY_HPP

#include <cstddef>
#include <cstdint>
#include <functional>
#include <utility>

#include "utils/assert.hpp"  // IWYU pragma:  keep

namespace cft {
template <typename T, std::size_t Nm, typename Comp = std::less<T>>
struct SortedArray : Comp /*EBO*/ {
    using iterator        = T*;
    using size_type       = uint32_t;
    using value_type      = T;
    using reference       = T&;
    using pointer         = T*;
    using difference_type = std::ptrdiff_t;

    // NOTE: this is a workaround for the fact that in general we cannot have EBO while keeping
    // SortedArray an aggregate
    SortedArray(Comp comp = {})
        : Comp(comp) {
    }

    T        data[Nm];
    uint32_t sz = 0ULL;

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

    bool compare(T const& a, T const& b) const {
        return Comp::operator()(a, b);
    }

    template <typename U>
    bool try_insert(U&& elem) {

        if (sz >= Nm) {
            if (!compare(elem, back()))
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
        while (i > 0 && compare(elem, data[i - 1])) {
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

// NOTE: here for now, might be useful in other places, move to a more general header in case
template <typename T>
using no_cvr = typename std::remove_reference<typename std::remove_cv<T>::type>::type;

template <typename T, std::size_t Nm, typename Comp>
inline SortedArray<T, Nm, no_cvr<Comp>> make_custom_compare_sorted_array(Comp comparator) {
    return SortedArray<T, Nm, no_cvr<Comp>>{comparator};
}

}  // namespace cft
#endif /*CFT_SRC_CORE_SORTED_ARRAY_HPP*/
