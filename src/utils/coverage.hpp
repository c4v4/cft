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

#ifndef CFT_SRC_CORE_COVERAGE_HPP
#define CFT_SRC_CORE_COVERAGE_HPP


#include <cassert>
#include <cstddef>
#include <vector>

#include "core/cft.hpp"

namespace cft {

// Data structure to keep track of the number of times an element is covered by a set of sets of
// elements.
template <typename CounterT = uint16_t>
struct CoverCounters {
    using counter_t = CounterT;

    std::vector<counter_t> cov_counters;

    CoverCounters(size_t nelems = 0)
        : cov_counters(nelems, 0) {
    }

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

    // Check if all the elements of a subset are already covered.
    template <typename IterableT>
    bool is_redundant_cover(IterableT const& subset) const {
        for (auto i : subset)
            if (cov_counters[i] == 0)
                return false;
        return true;
    }

    // Check if all the elements would still be covered if the subset was removed.
    template <typename IterableT>
    bool is_redundant_uncover(IterableT const& subset) const {
        for (auto i : subset)
            if (cov_counters[i] <= 1)
                return false;
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
