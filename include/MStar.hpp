#ifndef SCP_INCLUDE_MSTAR_HPP_
#define SCP_INCLUDE_MSTAR_HPP_

#include "IndexList.hpp"
#include "cft.hpp"
class MStar {
public:
    MStar() = default;

    explicit MStar(idx_t nrows) : M_star(nrows, 0), zeros(nrows) { }

    inline void reset_uncovered(idx_t nrows) {
        zeros = nrows;
        M_star.assign(nrows, 0);
    }

    /// inline void reset_covered(const std::vector<Column>& cols, idx_t nrows) {
    template<typename Collection>
    inline void reset_covered(const Collection& cols, idx_t nrows) {
        reset_uncovered(nrows);
        for (auto& j_col : cols) { cover_rows(j_col); }
    }

    /// inline void reset_covered(const std::vector<Column>& cols, const std::vector<idx_t>& indexes, idx_t nrows) {
    template<typename Collection>
    inline void reset_covered(const Collection& cols, const std::vector<idx_t>& indexes, idx_t nrows) {
        reset_uncovered(nrows);
        for (auto& j : indexes) { cover_rows(cols[j]); }
    }

    inline bool is_redundant(const SubInstCol& col) {
        for (const auto i : col) {
            assert(M_star[i] > 0);
            if (M_star[i] == 1) { return false; }
        }
        return true;
    }

    template<typename IterableList>
    inline void cover_rows(const IterableList& rows) {
        for (auto i : rows) {
            assert(i < M_star.size());
            cover(i);
        }
    }

    template<typename IterableList>
    inline void uncover_rows(const IterableList& rows) {
        for (auto i : rows) {
            assert(i < M_star.size());
            uncover(i);
        }
    }

    inline void cover(idx_t row) {
        zeros -= static_cast<idx_t>(M_star[row] == 0);
        ++M_star[row];
    }

    inline void uncover(idx_t row) {
        zeros += static_cast<idx_t>(M_star[row] == 1);
        --M_star[row];
    }

    [[nodiscard]] idx_t get(idx_t idx) const { return M_star[idx]; }
    idx_t operator[](idx_t idx) const { return M_star[idx]; }
    idx_t& operator[](idx_t idx) { return M_star[idx]; }

    [[nodiscard]] inline auto begin() const { return M_star.begin(); }
    [[nodiscard]] inline auto end() const { return M_star.end(); }
    inline auto begin() { return M_star.begin(); }
    inline auto end() { return M_star.end(); }

    [[nodiscard]] idx_t get_uncovered() const { return zeros; }
    [[nodiscard]] idx_t get_covered() const { return M_star.size() - zeros; }

    [[nodiscard]] idx_t size() const { return M_star.size(); }


    std::vector<idx_t> M_star;
    idx_t zeros{};
};

#endif