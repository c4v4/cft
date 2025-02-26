// SPDX-FileCopyrightText: 2024 Francesco Cavaliere <francescocava95@gmail.com>
// SPDX-License-Identifier: MIT

#ifndef CFT_SRC_CORE_SPAN_HPP
#define CFT_SRC_CORE_SPAN_HPP


#include <cstddef>
#include <iterator>

#include "utils/assert.hpp"  // IWYU pragma:  keep
#include "utils/utility.hpp"

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

    size_type size() const {
        assert(finish >= start);
        return checked_cast<size_t>(finish - start);
    }

    bool empty() const {
        assert(finish >= start);
        return start == finish;
    }

    reference back() const {
        assert(start < finish);
        return *(finish - 1);
    }

    iterator begin() const {
        assert(finish >= start);
        return start;
    }

    iterator end() const {
        assert(finish >= start);
        return finish;
    }

    ret_type operator[](size_type i) const {
        assert(finish > start);
        assert(i < size());
        return start[i];
    }
};

template <typename ItT>
inline Span<ItT> make_span(ItT beg, ItT end) {
    assert(beg <= end);
    return {beg, end};
}

template <typename ItT>
inline Span<ItT> make_span(ItT beg, size_t sz) {
    return {beg, beg + sz};
}

}  // namespace cft


#endif /* CFT_SRC_CORE_SPAN_HPP */
