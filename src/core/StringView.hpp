// Copyright (c) 2024 Francesco Cavaliere
// This program is free software: you can redistribute it and/or modify it under the terms of the
// GNU General Public License as published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version. This program is distributed in the hope that it
// will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details. You should
// have received a copy of the GNU General Public License along with this program. If not, see
// <https://www.gnu.org/licenses/>.

#ifndef CAV_INCLUDE_STRING_VIEW_HPP
#define CAV_INCLUDE_STRING_VIEW_HPP

#include <cassert>
#include <cstddef>
#include <cstring>

#include "core/cft.hpp"

namespace cft {

struct StringView {
    using iterator        = char const*;
    using size_type       = std::size_t;
    using value_type      = char;
    using reference       = char&;
    using pointer         = char const*;
    using difference_type = std::ptrdiff_t;

    iterator start;
    iterator finish;

    CFT_NODISCARD size_type size() const noexcept {
        return finish - start;
    }

    CFT_NODISCARD bool empty() const noexcept {
        return start == finish;
    }

    CFT_NODISCARD pointer data() const noexcept {
        return start;
    }

    CFT_NODISCARD iterator begin() const noexcept {
        return start;
    }

    CFT_NODISCARD iterator end() const noexcept {
        return finish;
    }

    CFT_NODISCARD value_type operator[](size_type i) const noexcept {
        assert(finish > start);
        assert(i < static_cast<size_t>(finish - start));
        return start[i];
    }

    CFT_NODISCARD StringView remove_prefix(size_type n) const noexcept {
        assert(n <= size());
        return {start + n, finish};
    }

    CFT_NODISCARD StringView remove_suffix(size_type n) const noexcept {
        assert(n <= size());
        return {start, start + n};
    }

    CFT_NODISCARD StringView get_substr(size_type b, size_type e) const noexcept {
        assert(b <= size());
        assert(e <= size());
        return {start + b, start + e};
    }

    template <typename T>
    CFT_NODISCARD size_t find_first_if(T cond) const noexcept {
        for (size_t i = 0; i < size(); ++i)
            if (cond((*this)[i]))
                return i;
        return size();
    }

    template <typename T>
    CFT_NODISCARD size_t find_last_if(T cond) const noexcept {
        for (size_t i = size() - 1; i > 0; --i)
            if (cond((*this)[i]))
                return i;
        if (cond((*this)[0]))
            return 0;
        return size();
    }

    CFT_NODISCARD int compare(StringView other) const noexcept {
        size_t str_size = std::min(size(), other.size());
        int    res      = _compare_cstr(start, other.start, str_size);
        if (res != 0)
            return res;
        return size() == other.size() ? 0 : (size() < other.size() ? -1 : 1);
    }

    CFT_NODISCARD bool operator==(StringView rhs) const noexcept {
        return compare(rhs) == 0;
    }

    CFT_NODISCARD bool operator!=(StringView rhs) const noexcept {
        return compare(rhs) != 0;
    }

    CFT_NODISCARD bool operator<(StringView rhs) const noexcept {
        return compare(rhs) < 0;
    }

    CFT_NODISCARD bool operator<=(StringView rhs) const noexcept {
        return compare(rhs) <= 0;
    }

    CFT_NODISCARD bool operator>(StringView rhs) const noexcept {
        return compare(rhs) > 0;
    }

    CFT_NODISCARD bool operator>=(StringView rhs) const noexcept {
        return compare(rhs) >= 0;
    }

private:
    CFT_NODISCARD static int _compare_cstr(char const* s1, char const* s2, size_t n) noexcept {
        for (; n != 0; ++s1, ++s2, --n) {
            if (*s1 < *s2)
                return -1;
            if (*s1 > *s2)
                return 1;
        }
        return 0;
    }
};

CFT_NODISCARD inline StringView make_string_view() {
    return {{}, {}};
}

CFT_NODISCARD inline StringView make_string_view(char const* beg, char const* end) {
    return {beg, end};
}

CFT_NODISCARD inline StringView make_string_view(char const* beg, size_t sz) {
    return {beg, beg + sz};
}

CFT_NODISCARD inline StringView make_string_view(std::string const& str) {
    return {str.data(), str.data() + str.size()};
}

CFT_NODISCARD inline StringView make_string_view(char const* str) {
    return {str, str + std::strlen(str)};
}

}  // namespace cft


#endif /* CAV_INCLUDE_STRING_VIEW_HPP */
