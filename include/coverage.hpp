#ifndef CFT_INCLUDE_COUNTSET_HPP
#define CFT_INCLUDE_COUNTSET_HPP


#include "SparseBinMat.hpp"
#include "cft.hpp"

namespace cft {

/// @brief Data structure to keep track of the number of times a row is covered by a set of columns.
template <typename IdxT>
struct CoverCounters {
    using counter_t = uint16_t;

    std::vector<counter_t> cov_counters;

    void reset(IdxT nrows) {
        cov_counters.assign(nrows, 0);
    }

    template <typename IterableT>
    IdxT cover(IterableT const& col) {
        IdxT covered = 0;
        for (auto i : col) {
            assert(i < cov_counters.size());
            covered += cov_counters[i] == 0 ? 1 : 0;
            ++cov_counters[i];
        }
        return covered;
    }

    template <typename IterableT>
    IdxT uncover(IterableT const& col) {
        IdxT uncovered = 0;
        for (auto i : col) {
            assert(i < cov_counters.size());
            assert(cov_counters[i] > 0);
            --cov_counters[i];
            uncovered += cov_counters[i] == 0 ? 1 : 0;
        }
        return uncovered;
    }

    template <typename IterableT>
    CFT_NODISCARD bool is_redundant(IterableT const& col) {
        for (IdxT i : col)
            if (cov_counters[i] == 1)
                return false;
        return true;
    }

    CFT_NODISCARD counter_t operator[](IdxT i) const {
        assert(i < cov_counters.size());
        return cov_counters[i];
    }

    CFT_NODISCARD IdxT size() const {
        return cov_counters.size();
    }
};

template <typename IdxT>
CFT_NODISCARD inline CoverCounters<IdxT> make_cover_counters(IdxT nrows) {
    return {std::vector<uint16_t>(nrows)};
}

/// @brief Data structure to keep track what rows are covered by a set of columns.
template <typename IdxT>
struct CoverBits {
    std::vector<bool> cov_flags;

    void reset(IdxT nrows) {
        cov_flags.assign(nrows, false);
    }

    template <typename IterableT>
    IdxT cover(IterableT const& col) {
        IdxT covered = 0;
        for (auto i : col) {
            assert(i < cov_flags.size());
            bool was_covered = cov_flags[i];
            cov_flags[i]     = true;
            covered += was_covered ? 0 : 1;
        }
        return covered;
    }

    template <typename IterableT>
    IdxT uncover(IterableT const& col) {
        IdxT uncovered = 0;
        for (auto i : col) {
            assert(i < cov_flags.size());
            bool was_covered = cov_flags[i];
            cov_flags[i]     = false;
            uncovered += was_covered ? 1 : 0;
        }
        return uncovered;
    }

    CFT_NODISCARD bool operator[](IdxT i) const {
        assert(i < cov_flags.size());
        return cov_flags[i];
    }

    CFT_NODISCARD IdxT size() const {
        return cov_flags.size();
    }
};

template <typename IdxT>
CFT_NODISCARD inline CoverBits<IdxT> make_cover_bits(IdxT nrows) {
    return {std::vector<bool>(nrows)};
}

}  // namespace cft


#endif /* CFT_INCLUDE_COUNTSET_HPP */
