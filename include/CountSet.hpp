#ifndef CFT_INCLUDE_COUNTSET_HPP
#define CFT_INCLUDE_COUNTSET_HPP


#include "SparseBinMat.hpp"
#include "cft.hpp"

namespace cft {

struct CountSet {
    using counter_t = uint16_t;

    std::vector<counter_t> cov_counters;

    CFT_NODISCARD static CountSet create() {
        return {{}};
    }

    void reset_uncovered(idx_t nrows) {
        cov_counters.assign(nrows, 0);
    }

    idx_t reset_covered(SparseBinMat const& cols, idx_t nrows) {
        reset_uncovered(nrows);
        idx_t covered = 0;
        for (idx_t i : cols.idxs)
            covered += cover(i) ? 1 : 0;
        return covered;
    }

    template <typename ColsT, typename IdxsT>
    idx_t reset_covered(ColsT const& cols, IdxsT const& idxs, idx_t nrows) {
        reset_uncovered(nrows);
        idx_t covered = 0;
        for (auto const& j : idxs)
            covered += cover_rows(cols[j]);
        return covered;
    }

    template <typename IdxT>
    idx_t cover_rows(Span<IdxT*> col) {
        idx_t covered = 0;
        for (auto i : col)
            covered += cover(i) ? 1 : 0;
        return covered;
    }

    template <typename IdxT>
    idx_t uncover_rows(Span<IdxT*> col) {
        idx_t uncovered = 0;
        for (auto i : col)
            uncovered += uncover(i) ? 1 : 0;
        return uncovered;
    }

    bool cover(idx_t i) {
        assert(i < cov_counters.size());
        ++cov_counters[i];
        return cov_counters[i] == 1;
    }

    bool uncover(idx_t i) {
        assert(i < cov_counters.size());
        --cov_counters[i];
        return cov_counters[i] == 0;
    }

    template <typename IdxT>
    CFT_NODISCARD bool is_redundant(Span<IdxT*> col) {
        for (idx_t i : col)
            if (cov_counters[i] == 1)
                return false;
        return true;
    }

    CFT_NODISCARD counter_t operator[](idx_t i) const {
        assert(i < cov_counters.size());
        return cov_counters[i];
    }

    CFT_NODISCARD idx_t size() const {
        return cov_counters.size();
    }
};

struct BitSet {
    std::vector<bool> cov_flags;

    CFT_NODISCARD static CountSet create() {
        return {{}};
    }

    void reset_uncovered(idx_t nrows) {
        cov_flags.assign(nrows, false);
    }

    idx_t reset_covered(SparseBinMat const& cols, idx_t nrows) {
        reset_uncovered(nrows);
        idx_t covered = 0;
        for (idx_t i : cols.idxs)
            covered += cover(i) ? 1 : 0;
        return covered;
    }

    template <typename ColsT, typename IdxsT>
    idx_t reset_covered(ColsT const& cols, IdxsT const& idxs, idx_t nrows) {
        reset_uncovered(nrows);
        idx_t covered = 0;
        for (auto const& j : idxs)
            covered += cover_rows(cols[j]);
        return covered;
    }

    template <typename IdxT>
    idx_t cover_rows(Span<IdxT*> col) {
        idx_t covered = 0;
        for (auto i : col)
            covered += cover(i) ? 1 : 0;
        return covered;
    }

    template <typename IdxT>
    idx_t uncover_rows(Span<IdxT*> col) {
        idx_t uncovered = 0;
        for (auto i : col)
            uncovered += uncover(i) ? 1 : 0;
        return uncovered;
    }

    bool cover(idx_t i) {
        assert(i < cov_flags.size());
        bool was_covered = cov_flags[i];
        cov_flags[i]     = true;
        return !was_covered;
    }

    bool uncover(idx_t i) {
        assert(i < cov_flags.size());
        bool was_covered = cov_flags[i];
        cov_flags[i]     = false;
        return was_covered;
    }

    CFT_NODISCARD bool operator[](idx_t i) const {
        assert(i < cov_flags.size());
        return cov_flags[i];
    }

    CFT_NODISCARD idx_t size() const {
        return cov_flags.size();
    }
};
}  // namespace cft


#endif /* CFT_INCLUDE_COUNTSET_HPP */
