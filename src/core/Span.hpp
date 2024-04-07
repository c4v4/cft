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

#ifndef CFT_SRC_CORE_SPAN_HPP
#define CFT_SRC_CORE_SPAN_HPP


#include <bits/iterator_concepts.h>

#include <cassert>
#include <cstddef>
#include <iterator>

#include "core/cft.hpp"

namespace cft {

// A simple non-owning span
template <typename ItT>
struct Span {
    using iterator        = ItT;
    using size_type       = std::size_t;
    using ret_type        = decltype(std::declval<ItT>()[0]);  // does not discard constness
    using value_type      = typename std::iterator_traits<ItT>::value_type;
    using reference       = typename std::iterator_traits<ItT>::reference;
    using pointer         = typename std::iterator_traits<ItT>::pointer;
    using difference_type = typename std::iterator_traits<ItT>::difference_type;

    iterator start;
    iterator finish;

    CFT_NODISCARD size_type size() const {
        return finish - start;
    }

    CFT_NODISCARD bool empty() const {
        return start == finish;
    }

    CFT_NODISCARD reference back() const {
        assert(start != finish);
        return *(finish - 1);
    }

    CFT_NODISCARD iterator begin() const {
        return start;
    }

    CFT_NODISCARD iterator end() const {
        return finish;
    }

    CFT_NODISCARD ret_type operator[](size_type i) const {
        assert(finish > start);
        assert(i < static_cast<size_t>(finish - start));
        return start[i];
    }
};

template <typename ItT>
CFT_NODISCARD inline Span<ItT> make_span(ItT beg, ItT end) {
    return {beg, end};
}

template <typename ItT>
CFT_NODISCARD inline Span<ItT> make_span(ItT beg, size_t sz) {
    return {beg, beg + sz};
}

}  // namespace cft


#endif /* CFT_SRC_CORE_SPAN_HPP */
