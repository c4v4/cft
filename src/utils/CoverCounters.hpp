// SPDX-FileCopyrightText: 2024 Francesco Cavaliere <francescocava95@gmail.com>
// SPDX-License-Identifier: MIT

#ifndef CFT_SRC_CORE_COVERAGE_HPP
#define CFT_SRC_CORE_COVERAGE_HPP


#include <cstddef>
#include <cstdint>
#include <vector>

#include "utils/assert.hpp"  // IWYU pragma:  keep
#ifndef NDEBUG
#include "utils/utility.hpp"
#endif
namespace cft {

// Data structure to keep track of the number of times an element is covered (i.e., seen) by a set
// of sets of elements.
struct CoverCounters {
    using counter_t = uint32_t;

    std::vector<counter_t> cov_counters;

    explicit CoverCounters(size_t nelems = 0)
        : cov_counters(nelems, 0) {
    }

    void reset(size_t nelems) {
        cov_counters.assign(nelems, 0);
    }

    template <typename IterableT>
    size_t cover(IterableT const& subset) {
        size_t covered = 0;
        for (auto i : subset) {
            assert(checked_cast<size_t>(i) < cov_counters.size());
            covered += cov_counters[i] == 0 ? 1ULL : 0ULL;
            ++cov_counters[i];
        }
        return covered;
    }

    template <typename IterableT>
    size_t uncover(IterableT const& subset) {
        size_t uncovered = 0;
        for (auto i : subset) {
            assert(checked_cast<size_t>(i) < cov_counters.size());
            assert(cov_counters[i] > 0);
            --cov_counters[i];
            uncovered += cov_counters[i] == 0 ? 1ULL : 0ULL;
        }
        return uncovered;
    }

    // Check if all the elements of a subset are already covered.
    template <typename IterableT>
    bool is_redundant_cover(IterableT const& subset) const {
        for (auto i : subset) {
            assert(checked_cast<size_t>(i) < cov_counters.size());
            if (cov_counters[i] == 0)
                return false;
        }
        return true;
    }

    // Check if all the elements would still be covered if the subset was removed.
    template <typename IterableT>
    bool is_redundant_uncover(IterableT const& subset) const {
        for (auto i : subset) {
            assert(checked_cast<size_t>(i) < cov_counters.size());
            if (cov_counters[i] <= 1)
                return false;
        }
        return true;
    }

    counter_t operator[](size_t i) const {
        assert(i < cov_counters.size());
        return cov_counters[i];
    }

    size_t size() const {
        return cov_counters.size();
    }
};
}  // namespace cft


#endif /* CFT_SRC_CORE_COVERAGE_HPP */
