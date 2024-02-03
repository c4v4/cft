#ifndef SCP_INCLUDE_GREEDY_HPP_
#define SCP_INCLUDE_GREEDY_HPP_

#include <algorithm>
#include <cassert>
#include <vector>

#include "CountSet.hpp"
#include "Instance.hpp"
#include "RedundancyRemoval.hpp"
#include "Scores.hpp"
#include "Solution.hpp"
#include "cft.hpp"

/**
 * @brief (It goes purposelly against RAII to maintain the memory allocations between subsequent
 * calls)
 *
 */
class Greedy {
public:
    Greedy(SubInstance& subinst_)
        : subinst(subinst_)
        , sigmas(subinst_)
        , remove_redundat_cols(subinst_) {
    }

    /**
     * @brief Greedy procedure that finds feasible solutions starting from a set of Lagrangian
     * multipliers.
     *
     * @param u_k   Array of Lagrangian multipliers
     * @param S     LocalSolution
     */
    LocalSolution operator()(LocalMultipliers const& u_k) {
        LocalSolution S;
        operator()(u_k, std::numeric_limits<idx_t>::max(), S);
        return S;
    }

    void operator()(LocalMultipliers const& u_k, idx_t Ssize, LocalSolution& S) {

        BitSet& covered_rows = subinst.get_covered_rows();
        covered_rows.reset_covered(subinst.get_cols(), S, subinst.get_nrows());

        // Score initialization
        sigmas.setup_scores(u_k, covered_rows);

        while (covered_rows.get_uncovered() > 0 && S.size() < Ssize) {
            idx_t j_star = sigmas.argmin_score();             // Step 2
            sigmas.update_scores(u_k, j_star, covered_rows);  // Step 2'

            assert(std::find(S.begin(), S.end(), j_star) == S.end());

            covered_rows.cover_rows(subinst.get_col(j_star));
            S.emplace_back(j_star);  // Step 3
        }

        IF_DEBUG {
            assert(covered_rows.get_covered() == subinst.get_nrows() || S.size() >= Ssize);
            covered_rows.reset_covered(subinst.get_cols(), S, subinst.get_nrows());
            if (S.size() < Ssize)
                assert(covered_rows.get_uncovered() == 0);
        }

        CountSet& covering_times = subinst.get_covering_times();
        covering_times.reset_covered(subinst.get_cols(), S, subinst.get_nrows());
        remove_redundat_cols(S, u_k, covering_times);

        IF_DEBUG {
            covering_times.reset_covered(subinst.get_cols(), S, subinst.get_nrows());
            if (S.size() < Ssize)
                assert(covering_times.get_uncovered() == 0);
        }
    }

private:
    SubInstance&      subinst;
    Scores            sigmas;
    RendundacyRemoval remove_redundat_cols;
};


#endif