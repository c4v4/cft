#ifndef SCP_INCLUDE_COLUMNFIXING_HPP_
#define SCP_INCLUDE_COLUMNFIXING_HPP_

#include "Greedy.hpp"
#include "Instance.hpp"
#include "cft.hpp"

#define Q_THRESHOLD (-0.001)

class ColumnFixing {
public:
    ColumnFixing(SubInstance& subinst_, Greedy& greedy_)
        : subinst(subinst_)
        , greedy(greedy_) {
    }

    idx_t operator()(LocalMultipliers& u_star, GlobalSolution& S_star) {
        auto& cols  = subinst.get_cols();
        auto  nrows = subinst.get_nrows();

        std::sort(S_star.begin(), S_star.end());

        Q.clear();
        for (idx_t j = 0; auto& col : cols) {
            if (col.compute_lagr_cost(u_star) < Q_THRESHOLD &&
                std::binary_search(S_star.begin(), S_star.end(), subinst.get_global_col_idx(j))) {
                Q.emplace_back(j);
            }
            ++j;
        }

        CountSet& covering_times = subinst.get_covering_times();
        covering_times.reset_covered(cols, Q, nrows);

        columns_to_fix.clear();
        for (auto j : Q)
            if (!covering_times.is_redundant(cols[j]))
                columns_to_fix.emplace_back(j);

        LocalSolution S(columns_to_fix);
        greedy(u_star, std::max<idx_t>(nrows / 200, 1UL), S);

        idx_t remaining_rows = subinst.fix_columns(S, u_star);

        // IF_VERBOSE { fmt::print("Fixed {} columns.\n", S.size()); }

        IF_DEBUG {
            assert(!subinst.is_corrupted());
            for ([[maybe_unused]] auto& col : cols)
                assert(!col.empty());
        }

        return remaining_rows;
    }

private:
    SubInstance& subinst;
    Greedy&      greedy;

    std::vector<idx_t> Q;
    std::vector<idx_t> columns_to_fix;
};

#endif