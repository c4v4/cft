#ifndef SCP_INCLUDE_MSTAR_HPP_
#define SCP_INCLUDE_MSTAR_HPP_

#include "CollectionOf.hpp"
#include "cft.hpp"

class CountSet {
public:
    CountSet() = default;

    explicit CountSet(idx_t nrows)
        : covering_times(nrows, 0)
        , zeros(nrows) {
    }

    inline void reset_uncovered(idx_t nrows) {
        zeros = nrows;
        covering_times.assign(nrows, 0);
    }

    /// inline void reset_covered(const std::vector<Column>& cols, idx_t nrows) {
    template <typename Collection>
    inline void reset_covered(Collection const& cols, idx_t nrows) {
        reset_uncovered(nrows);
        for (auto& j_col : cols)
            cover_rows(j_col);
    }

    /// inline void reset_covered(const std::vector<Column>& cols, const std::vector<idx_t>&
    /// indexes, idx_t nrows) {
    template <typename Collection>
    inline void reset_covered(Collection const&         cols,
                              std::vector<idx_t> const& indexes,
                              idx_t                     nrows) {
        reset_uncovered(nrows);
        for (auto& j : indexes)
            cover_rows(cols[j]);
    }

    inline bool is_redundant(SubInstCol const& col) {
        for (auto const i : col) {
            assert(covering_times[i] > 0);
            if (covering_times[i] == 1)
                return false;
        }
        return true;
    }

    template <typename IterableList>
    inline void cover_rows(IterableList const& rows) {
        for (auto i : rows) {
            assert(i < covering_times.size());
            cover(i);
        }
    }

    template <typename IterableList>
    inline void uncover_rows(IterableList const& rows) {
        for (auto i : rows) {
            assert(i < covering_times.size());
            uncover(i);
        }
    }

    inline void cover(idx_t row) {
        zeros -= static_cast<idx_t>(covering_times[row] == 0);
        ++covering_times[row];
    }

    inline void uncover(idx_t row) {
        zeros += static_cast<idx_t>(covering_times[row] == 1);
        --covering_times[row];
    }

    [[nodiscard]] auto get(idx_t idx) const {
        return covering_times[idx];
    }

    auto operator[](idx_t idx) const {
        return covering_times[idx];
    }

    decltype(auto) operator[](idx_t idx) {
        return covering_times[idx];
    }

    [[nodiscard]] inline auto begin() const {
        return covering_times.begin();
    }

    [[nodiscard]] inline auto end() const {
        return covering_times.end();
    }

    inline auto begin() {
        return covering_times.begin();
    }

    inline auto end() {
        return covering_times.end();
    }

    [[nodiscard]] idx_t get_uncovered() const {
        return zeros;
    }

    [[nodiscard]] idx_t get_covered() const {
        return covering_times.size() - zeros;
    }

    [[nodiscard]] idx_t size() const {
        return covering_times.size();
    }


private:
    std::vector<uint16_t> covering_times;
    idx_t                 zeros{};
};

class BitSet {
public:
    BitSet() = default;

    explicit BitSet(idx_t nrows)
        : bit_set(nrows) {
    }

    inline void reset_uncovered(idx_t nrows) {
        ntrue = 0;
        bit_set.assign(nrows, false);
    }

    template <typename Collection>
    inline void reset_covered(Collection const& cols, idx_t nrows) {
        reset_uncovered(nrows);
        for (auto& j_col : cols)
            cover_rows(j_col);
    }

    /// inline void reset_covered(const std::vector<Column>& cols, const std::vector<idx_t>&
    /// indexes, idx_t nrows) {
    template <typename Collection>
    inline void reset_covered(Collection const&         cols,
                              std::vector<idx_t> const& indexes,
                              idx_t                     nrows) {
        reset_uncovered(nrows);
        for (idx_t j : indexes)
            cover_rows(cols[j]);
    }

    template <typename IterableList>
    inline void cover_rows(IterableList const& rows) {
        for (auto i : rows) {
            assert(i < bit_set.size());
            cover(i);
        }
    }

    template <typename IterableList>
    inline void uncover_rows(IterableList const& rows) {
        for (auto i : rows) {
            assert(i < bit_set.size());
            uncover(i);
        }
    }

    inline void cover(idx_t row) {
        ntrue += bit_set[row] ? 0 : 1;
        bit_set[row] = true;
    }

    inline void uncover(idx_t row) {
        ntrue += bit_set[row] ? -1 : 0;
        bit_set[row] = true;
    }

    [[nodiscard]] decltype(auto) get(idx_t idx) const {
        return bit_set[idx];
    }

    decltype(auto) operator[](idx_t idx) const {
        return bit_set[idx];
    }

    decltype(auto) operator[](idx_t idx) {
        return bit_set[idx];
    }

    [[nodiscard]] inline auto begin() const {
        return bit_set.begin();
    }

    [[nodiscard]] inline auto end() const {
        return bit_set.end();
    }

    inline auto begin() {
        return bit_set.begin();
    }

    inline auto end() {
        return bit_set.end();
    }

    [[nodiscard]] idx_t get_uncovered() const {
        return bit_set.size() - ntrue;
    }

    [[nodiscard]] idx_t get_covered() const {
        return ntrue;
    }

    [[nodiscard]] idx_t size() const {
        return bit_set.size();
    }


private:
    std::vector<bool> bit_set;
    idx_t             ntrue = {};
};

#endif