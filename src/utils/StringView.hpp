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

#ifndef CFT_SRC_CORE_STRINGVIEW_HPP
#define CFT_SRC_CORE_STRINGVIEW_HPP


#include <cassert>
#include <cstddef>
#include <cstring>
#include <string>

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

    StringView() = default;

    StringView(char const* beg, char const* end)
        : start(beg)
        , finish(end) {
    }

    StringView(char const* beg, size_t sz)
        : start(beg)
        , finish(beg + sz) {
    }

    StringView(std::string const& str)
        : start(str.data())
        , finish(str.data() + str.size()) {
    }

    StringView(char const* str)
        : start(str)
        , finish(str + std::strlen(str)) {
    }

    size_type size() const noexcept {
        return finish - start;
    }

    bool empty() const noexcept {
        return start == finish;
    }

    pointer data() const noexcept {
        return start;
    }

    iterator begin() const noexcept {
        return start;
    }

    iterator end() const noexcept {
        return finish;
    }

    value_type operator[](size_type i) const noexcept {
        assert(finish > start);
        assert(i < static_cast<size_t>(finish - start));
        return start[i];
    }

    StringView remove_prefix(size_type n) const noexcept {
        assert(n <= size());
        return {start + n, finish};
    }

    StringView remove_suffix(size_type n) const noexcept {
        assert(n <= size());
        return {start, start + n};
    }

    StringView get_substr(size_type b, size_type e) const noexcept {
        assert(b <= size());
        assert(e <= size());
        return {start + b, start + e};
    }

    template <typename T>
    size_t find_first_if(T cond) const noexcept {
        for (size_t i = 0; i < size(); ++i)
            if (cond((*this)[i]))
                return i;
        return size();
    }

    template <typename T>
    size_t find_last_if(T cond) const noexcept {
        for (size_t i = size() - 1; i > 0; --i)
            if (cond((*this)[i]))
                return i;
        if (cond((*this)[0]))
            return 0;
        return size();
    }

    int compare(StringView other) const noexcept {
        size_t str_size = std::min(size(), other.size());
        int    res      = _compare_cstr(start, other.start, str_size);
        if (res != 0)
            return res;
        return size() == other.size() ? 0 : (size() < other.size() ? -1 : 1);
    }

    bool operator==(StringView rhs) const noexcept {
        return compare(rhs) == 0;
    }

    bool operator!=(StringView rhs) const noexcept {
        return compare(rhs) != 0;
    }

    bool operator<(StringView rhs) const noexcept {
        return compare(rhs) < 0;
    }

    bool operator<=(StringView rhs) const noexcept {
        return compare(rhs) <= 0;
    }

    bool operator>(StringView rhs) const noexcept {
        return compare(rhs) > 0;
    }

    bool operator>=(StringView rhs) const noexcept {
        return compare(rhs) >= 0;
    }

private:
    static int _compare_cstr(char const* s1, char const* s2, size_t n) noexcept {
        for (; n != 0; ++s1, ++s2, --n) {
            if (*s1 < *s2)
                return -1;
            if (*s1 > *s2)
                return 1;
        }
        return 0;
    }
};

}  // namespace cft


#endif /* CFT_SRC_CORE_STRINGVIEW_HPP */
