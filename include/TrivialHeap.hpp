#ifndef SCP_INCLUDE_TRIVIALHEAP_HPP_
#define SCP_INCLUDE_TRIVIALHEAP_HPP_

#include <array>
#include <cinttypes>
#include <functional>

template <typename T, std::size_t Nm, typename Comp = std::less<T>>
class TrivialHeap {
public:
    inline auto size() const { return sz; }

    inline void pop_back() {
        assert(sz > 0);
        --sz;
    }

    inline void push_back(T elem) {  // delete me
        assert(sz < Nm);
        buf[sz] = elem;
        ++sz;
    }

    inline auto back() const {
        assert(sz > 0);
        return buf[sz - 1];
    }

    inline auto& back() {
        assert(sz > 0);
        return buf[sz - 1];
    }

    inline auto begin() const { return std::addressof(buf[0]); }
    inline auto begin() { return std::addressof(buf[0]); }
    inline auto end() const { return std::addressof(buf[sz]); }
    inline auto end() { return std::addressof(buf[sz]); }

    inline bool try_insert(T elem) {

        if (sz >= Nm) {
            if (!Comp()(back(), elem)) { return false; }
            pop_back();
        }

        insert(elem);
        return true;
    }

    inline void insert(T elem) {
        assert(sz < Nm);

        auto* ptr = end();
        while (ptr != begin() && Comp()(*(ptr - 1), elem)) {
            *(ptr) = *(ptr - 1);
            --ptr;
        }
        *ptr = elem;
        ++sz;
    }

    inline void clear() { sz = 0U; }

private:
    T buf[Nm];
    uint32_t sz = 0U;
};

#endif