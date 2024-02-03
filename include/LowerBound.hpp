#ifndef SCP_INCLUDE_LOWERBOUND_HPP_
#define SCP_INCLUDE_LOWERBOUND_HPP_

#include <algorithm>
#include <array>
#include <cassert>
#include <vector>

#include "CountSet.hpp"
#include "Instance.hpp"
#include "Multipliers.hpp"
#include "Solution.hpp"
#include "cft.hpp"

/**
 * @brief Return the LB computed using the Lagrangian multipliers.
 *
 */
template <typename Iter, int n = 0, int d = 1>
real_t lagr_mul_LB(SubInstance&            subinst,
                   LocalMultipliers const& u_k,
                   Iter const&             begin,
                   Iter const&             end) {
    constexpr real_t threshold = static_cast<real_t>(n) / static_cast<real_t>(d);

    real_t LB = std::reduce(u_k.begin(), u_k.end(), 0.0);

    for (auto it = begin; it != end; ++it) {
        auto&  col = subinst.get_col(*it);
        real_t c_u = col.compute_lagr_cost(u_k);
        if (c_u < threshold)
            LB += c_u;
    }

    return LB;
}

template <int n = 0, int d = 1>  // seriously c++?
real_t lagr_mul_LB(SubInstance& subinst, std::vector<real_t> const& u_k) {
    constexpr real_t threshold = static_cast<real_t>(n) / static_cast<real_t>(d);

    auto&  cols = subinst.get_cols();
    real_t LB   = std::reduce(u_k.begin(), u_k.end(), 0.0);
    // fmt::print("BASE LB ORIG: {}\n", LB);
    for (auto& col : cols) {
        real_t c_u = col.compute_lagr_cost(u_k);
        if (c_u < threshold)
            LB += c_u;
    }

    return LB;
}

template <int n = 0, int d = 1>  // seriously c++?
real_t lagr_mul_LB(SubInstance const& subinst, std::vector<real_t> const& u_k) {
    constexpr real_t threshold = static_cast<real_t>(n) / static_cast<real_t>(d);

    auto const& cols = subinst.get_cols();
    real_t      LB   = std::reduce(u_k.begin(), u_k.end(), 0.0);
    // fmt::print("BASE LB ORIG: {}\n", LB);
    for (auto const& col : cols) {
        real_t c_u = col.compute_lagr_cost(u_k);
        if (c_u < threshold)
            LB += c_u;
    }

    return LB;
}

template <int n = 0, int d = 1>  // seriously c++?
class DeltaLowerBound {
    static constexpr real_t threshold = static_cast<real_t>(n) / static_cast<real_t>(d);

public:
    DeltaLowerBound()
        : LB(0.0) {
    }

    real_t compute(SubInstance& subinst, std::vector<real_t> const& u_k) {
        return LB = lagr_mul_LB<n, d>(subinst, u_k);
    }

    real_t update(SubInstance& subinst, std::vector<std::pair<idx_t, real_t>> const& delta_u_k) {

        auto& cols = subinst.get_cols();
        auto& rows = subinst.get_rows();
        // real_t base = 0.0;
        for (auto& [i, delta_ui] : delta_u_k) {
            LB += delta_ui;
            // base += delta_ui;
            for (auto j : rows[i]) {
                real_t old_c = cols[j].get_cu();
                real_t new_c = cols[j].update_cu(delta_ui);

                if (new_c < threshold)
                    LB += new_c;
                if (old_c < threshold)
                    LB -= old_c;
            }
        }

        // fmt::print("BASE LB DELT: {}\n", base);
        return LB;
    }

private:
    real_t LB;
};

template <int n = 1, int d = 1000>  // seriously c++?
class ReducedLagrMultLB {
    static constexpr real_t threshold = static_cast<real_t>(n) / static_cast<real_t>(d);

public:
    void compute_reduced_sol(SubInstance& subinst, LocalSolution& S, CountSet& I_S) {
        auto const& cols = subinst.get_cols();

        I_S.reset_uncovered(subinst.get_nrows());

        S.clear();
        for (idx_t j = 0; auto& col : cols) {
            if (col.get_cu() <= threshold) {
                S.emplace_back(j);
                I_S.cover_rows(col);
            }
            ++j;
        }

        R.clear();
        idx_t Ssize = S.size();
        for (idx_t sj = 0; sj < Ssize; ++sj)
            if (I_S.is_redundant(cols[S[sj]]))
                R.emplace_back(sj);

        std::sort(R.begin(), R.end(), [&](auto const sj1, auto const sj2) {
            return cols[S[sj1]].get_cu() > cols[S[sj2]].get_cu();
        });

        for (auto const j : R) {
            if (I_S.is_redundant(cols[S[j]])) {
                I_S.uncover_rows(cols[S[j]]);
                S.mark_removal(j);
            }
        }
        S.apply_removal();
    }

    void compute_sol(SubInstance& subinst, LocalSolution& S, CountSet& I_S) {
        auto const& cols = subinst.get_cols();

        S.clear();
        for (idx_t j = 0; auto& col : cols) {
            if (col.get_cu() <= threshold)
                S.emplace_back(j);
            ++j;
        }

        I_S.reset_covered(cols, S, subinst.get_nrows());
    }

private:
    std::vector<idx_t> R;
};

#endif