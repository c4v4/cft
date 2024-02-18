#ifndef CFT_INCLUDE_COUNTSET_HPP
#define CFT_INCLUDE_COUNTSET_HPP

#include <cassert>
#include <vector>

#include "cft.hpp"
#include "util_functions.hpp"

namespace cft {

/// @brief Data structure to keep track of the number of times an element is covered by a set of
/// sets of elements.
template <typename CounterT = uint16_t>
struct CoverCounters {
    using counter_t = CounterT;

    std::vector<counter_t> cov_counters;

    void reset(size_t nelems) {
        cov_counters.assign(nelems, 0);
    }

    template <typename IterableT>
    size_t cover(IterableT const& subset) {
        size_t covered = 0;
        for (auto i : subset) {
            assert(i < cov_counters.size());
            covered += cov_counters[i] == 0 ? 1 : 0;
            ++cov_counters[i];
        }
        return covered;
    }

    template <typename IterableT>
    size_t uncover(IterableT const& subset) {
        size_t uncovered = 0;
        for (auto i : subset) {
            assert(i < cov_counters.size());
            assert(cov_counters[i] > 0);
            --cov_counters[i];
            uncovered += cov_counters[i] == 0 ? 1 : 0;
        }
        return uncovered;
    }

    /// @brief Check if all the elements of a subset are already covered.
    template <typename IterableT>
    CFT_NODISCARD bool is_redundant_cover(IterableT const& subset) {
        for (auto i : subset)
            if (cov_counters[i] == 0)
                return false;
        return true;
    }

    /// @brief Check if all the elements would still be covered if the subset was removed.
    template <typename IterableT>
    CFT_NODISCARD bool is_redundant_uncover(IterableT const& subset) {
        for (auto i : subset)
            if (cov_counters[i] <= 1)
                return false;
        return true;
    }

    CFT_NODISCARD counter_t operator[](size_t i) const {
        assert(i < cov_counters.size());
        return cov_counters[i];
    }

    CFT_NODISCARD size_t size() const {
        return cov_counters.size();
    }
};

template <typename CounterT = uint16_t>
CFT_NODISCARD inline CoverCounters<CounterT> make_cover_counters(size_t nelems) {
    return {std::vector<CounterT>(nelems)};
}

/// @brief Data structure to keep track of what elements are covered by a set of sets of elements.
struct CoverBits {
    std::vector<bool> cov_flags;

    void reset(size_t nelems) {
        cov_flags.assign(nelems, false);
    }

    template <typename IterableT>
    size_t cover(IterableT const& subset) {
        size_t covered = 0;
        for (auto i : subset) {
            assert(i < cov_flags.size());
            bool was_covered = cov_flags[i];
            cov_flags[i]     = true;
            covered += was_covered ? 0 : 1;
        }
        return covered;
    }

    template <typename IterableT>
    size_t uncover(IterableT const& subset) {
        size_t uncovered = 0;
        for (auto i : subset) {
            assert(i < cov_flags.size());
            bool was_covered = cov_flags[i];
            cov_flags[i]     = false;
            uncovered += was_covered ? 1 : 0;
        }
        return uncovered;
    }

    /// @brief Check if all the elements of a subset are already covered.
    template <typename IterableT>
    CFT_NODISCARD bool is_redundant_cover(IterableT const& subset) {
        for (auto i : subset)
            if (!cov_flags[i])
                return false;
        return true;
    }

    CFT_NODISCARD bool operator[](size_t i) const {
        assert(i < cov_flags.size());
        return cov_flags[i];
    }

    CFT_NODISCARD size_t size() const {
        return cov_flags.size();
    }
};

CFT_NODISCARD inline CoverBits make_cover_bits(size_t nelems) {
    return {std::vector<bool>(nelems)};
}

}  // namespace cft


#endif /* CFT_INCLUDE_COUNTSET_HPP */
