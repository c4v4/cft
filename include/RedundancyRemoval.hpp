#ifndef SCP_INCLUDE_RENDUNDACYREMOVAL_HPP_
#define SCP_INCLUDE_RENDUNDACYREMOVAL_HPP_

#include <algorithm>
#include <array>
#include <cassert>
#include <vector>

#include "Instance.hpp"
#include "LowerBound.hpp"
#include "MStar.hpp"
#include "Scores.hpp"
#include "Solution.hpp"
#include "cft.hpp"

#define ENUM_THRESH 10

template <typename T, size_t N>
constexpr std::array<T, N> make_array(T value) {
    std::array<T, N> a;
    for (auto& x : a) { x = value; }
    return a;
}


class RendundacyRemoval {
public:
    RendundacyRemoval(SubInstance& subinst_) : subinst(subinst_) { }

    void reset(const LocalSolution& S, MStar& M_star) {
        redundant_cols.clear();
        for (auto j : S) {
            if (M_star.is_redundant(subinst.get_col(j))) { redundant_cols.emplace_back(j); }
        }
        uncovered_rows = M_star.get_uncovered();
    }

    inline auto size() { return redundant_cols.size(); }

    void _enumeration_removal(std::array<bool, ENUM_THRESH>& vars, idx_t end, MStar& M_star, real_t partial_cost, const LocalMultipliers& u_k, real_t& UB,
                              std::array<bool, ENUM_THRESH>& sol) {

        assert(M_star.get_uncovered() == uncovered_rows);

        if (end == 0) {
            if (partial_cost < UB) {
                UB = partial_cost;
                for (idx_t i = 0; i < ENUM_THRESH; ++i) sol[i] = vars[i];
            }
            return;
        }

        idx_t last = end - 1;
        auto& col = subinst.get_col(redundant_cols[last]);

        if (M_star.is_redundant(col)) {

            vars[last] = false;
            M_star.uncover_rows(col);
            assert(M_star.get_uncovered() == uncovered_rows);

            _enumeration_removal(vars, end - 1, M_star, partial_cost, u_k, UB, sol);

            vars[last] = true;
            M_star.cover_rows(col);
            assert(M_star.get_uncovered() == uncovered_rows);
        }

        if (partial_cost >= UB) { return; }

        auto next_p_cost = partial_cost + col.get_cost();
        if (next_p_cost >= UB) { return; }

        _enumeration_removal(vars, end - 1, M_star, next_p_cost, u_k, UB, sol);
    }

    auto heur_enum_removal(const LocalMultipliers& u_k, MStar& M_star) {
        // assert(M_star.get_uncovered() == uncovered_rows);

        cols_to_remove.clear();
        if (redundant_cols.empty()) { return cols_to_remove; }

        std::sort(redundant_cols.begin(), redundant_cols.end(), [&](auto a, auto b) { return subinst.get_col(a).get_cost() < subinst.get_col(b).get_cost(); });

        // IF_VERBOSE { fmt::print("Found {} redundant cols {{{}}}\n", redundant_cols.size(), fmt::join(redundant_cols, ", ")); }

        if (redundant_cols.size() > ENUM_THRESH) {
            // the first one is free
            cols_to_remove.emplace_back(redundant_cols.back());
            M_star.uncover_rows(subinst.get_col(redundant_cols.back()));
            redundant_cols.pop_back();

            auto& cols = subinst.get_cols();
            while (redundant_cols.size() > ENUM_THRESH) {
                auto j = redundant_cols.back();
                if (M_star.is_redundant(cols[j])) {
                    cols_to_remove.emplace_back(j);
                    for (auto i : subinst.get_col(j)) { M_star.uncover(i); }
                }
                redundant_cols.pop_back();
            }
        }

        if (!redundant_cols.empty()) {
            // start enumeration
            auto vars = make_array<bool, ENUM_THRESH>(true);

            real_t UB = REAL_MAX;  // enum first tries to remove all the column in order
            std::array<bool, ENUM_THRESH> sol;
            _enumeration_removal(vars, redundant_cols.size(), M_star, 0.0, u_k, UB, sol);

            for (idx_t j = 0; j < redundant_cols.size(); ++j) {
                if (!sol[j]) { cols_to_remove.emplace_back(redundant_cols[j]); }
            }
        }

        // IF_VERBOSE {
        //    real_t cost = std::accumulate(cols_to_remove.begin(), cols_to_remove.end(), 0.0,
        //                                  [&](auto sum, auto idx) { return sum + subinst.get_col(idx).get_cost(); });
        //    fmt::print("removed {} cols, cost improvement of {}: {{{}}}\n", cols_to_remove.size(), cost, fmt::join(cols_to_remove, ", "));
        //}

        return cols_to_remove;
    }

    NO_INLINE void operator()(LocalSolution& S, const LocalMultipliers& u_k, MStar& M_star) {
        reset(S, M_star);
        cols_to_remove = heur_enum_removal(u_k, M_star);
        S.remove(cols_to_remove);
    }

private:
    SubInstance& subinst;
    std::vector<idx_t> redundant_cols;
    std::vector<idx_t> cols_to_remove;
    idx_t uncovered_rows = 0;
};

#endif