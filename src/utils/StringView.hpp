// SPDX-FileCopyrightText: 2024 Francesco Cavaliere <francescocava95@gmail.com>
// SPDX-License-Identifier: MIT

#ifndef CFT_SRC_CORE_STRINGVIEW_HPP
#define CFT_SRC_CORE_STRINGVIEW_HPP


#include <cstddef>
#include <cstring>
#include <string>

#include "utils/assert.hpp"  // IWYU pragma:  keep
#include "utils/utility.hpp"

namespace cft {

// Specialized non-owning Span for strings
struct StringView {
    using iterator        = char const*;
    using size_type       = std::size_t;
    using value_type      = char;
    using reference       = char&;
    using pointer         = char const*;
    using difference_type = std::ptrdiff_t;

    iterator start;
    iterator finish;

    StringView() = default;

    StringView(char const* beg, char const* end)
        : start(beg)
        , finish(end) {
        assert(finish >= start);
    }

    StringView(char const* beg, size_t sz)
        : start(beg)
        , finish(beg + sz) {
        assert(finish >= start);
    }

    // The main consequence of making this constructor explicit is that the user would need
    // to provide overloads that accept string-like objects, explicitly or using templates.
    // The main advantage is that in this way they have to opt-in for the conversion (handling
    // the overloads), while with the implicit approach, the conversion is always possible. However,
    // considering the scope of this project and the use of this class, using the implicit approach
    // is acceptable and cleaner.
    // NOLINTNEXTLINE(hicpp-explicit-conversions, google-explicit-constructor)
    StringView(std::string const& str)
        : start(str.data())
        , finish(str.data() + str.size()) {
        assert(finish >= start);
    }

    // NOLINTNEXTLINE(hicpp-explicit-conversions, google-explicit-constructor)
    StringView(char const* str)
        : start(str)
        , finish(str + std::strlen(str)) {
        assert(finish >= start);
    }

    std::string to_cpp_string() const {
        assert(finish >= start);
        return {start, finish};
    }

    size_type size() const {
        assert(finish >= start);
        return checked_cast<size_t>(finish - start);
    }

    bool empty() const {
        assert(finish >= start);
        return start == finish;
    }

    pointer data() const {
        return start;
    }

    iterator begin() const {
        assert(finish >= start);
        return start;
    }

    iterator end() const {
        assert(finish >= start);
        return finish;
    }

    value_type operator[](size_type i) const {
        assert(i < size());
        return start[i];
    }

    StringView remove_prefix(size_type pos) const {
        assert(pos <= size());
        return {start + pos, finish};
    }

    StringView remove_suffix(size_type pos) const {
        assert(pos <= size());
        return {start, start + pos};
    }

    StringView get_substr(size_type beg_pos, size_type end_pos) const {
        assert(beg_pos <= size());
        assert(end_pos <= size());
        return {start + beg_pos, start + end_pos};
    }

    template <typename T>
    size_t find_first_true(T cond) const {
        for (size_t i = 0; i < size(); ++i)
            if (cond((*this)[i]))
                return i;
        return size();
    }

    template <typename T>
    size_t find_last_true(T cond) const {
        if (empty())
            return size();
        for (size_t i = size() - 1; i > 0; --i)
            if (cond((*this)[i]))
                return i;
        if (cond((*this)[0]))
            return 0;
        return size();
    }

    int compare(StringView other) const {
        size_t min_size = cft::min(size(), other.size());
        for (size_t n = 0; n < min_size; ++n) {
            if ((*this)[n] < other[n])
                return -1;
            if ((*this)[n] > other[n])
                return 1;
        }
        return size() < other.size() ? -1 : (size() > other.size() ? 1 : 0);
    }

    bool operator==(StringView rhs) const {
        return compare(rhs) == 0;
    }

    bool operator!=(StringView rhs) const {
        return compare(rhs) != 0;
    }

    bool operator<(StringView rhs) const {
        return compare(rhs) < 0;
    }

    bool operator<=(StringView rhs) const {
        return compare(rhs) <= 0;
    }

    bool operator>(StringView rhs) const {
        return compare(rhs) > 0;
    }

    bool operator>=(StringView rhs) const {
        return compare(rhs) >= 0;
    }
};

}  // namespace cft


#endif /* CFT_SRC_CORE_STRINGVIEW_HPP */
