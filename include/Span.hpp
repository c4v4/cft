#ifndef CFT_INCLUDE_SPAN_HPP
#define CFT_INCLUDE_SPAN_HPP

#include <cstddef>
#include <iterator>

#include "cft.hpp"

namespace cft {

/// @brief A simple non-owning span
template <typename ItT>
struct Span {
    using iterator        = ItT;
    using size_type       = std::size_t;
    using value_type      = typename std::iterator_traits<ItT>::value_type;
    using reference       = typename std::iterator_traits<ItT>::reference;
    using pointer         = typename std::iterator_traits<ItT>::pointer;
    using difference_type = typename std::iterator_traits<ItT>::difference_type;

    iterator start;
    iterator finish;

    CFT_NODISCARD static Span create() {
        return {{}, {}};
    }

    CFT_NODISCARD size_type size() const {
        return finish - start;
    }

    CFT_NODISCARD bool empty() const {
        return start == finish;
    }

    CFT_NODISCARD iterator begin() const {
        return start;
    }

    CFT_NODISCARD iterator end() const {
        return finish;
    }

    CFT_NODISCARD ItT const& operator[](size_type i) const {
        assert(i < finish - start);
        return start[i];
    }
};

}  // namespace cft

#endif /* CFT_INCLUDE_SPAN_HPP */
