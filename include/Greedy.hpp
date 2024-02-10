#ifndef SCP_INCLUDE_GREEDY_HPP_
#define SCP_INCLUDE_GREEDY_HPP_

#include <algorithm>
#include <cassert>
#include <vector>

#include "Instance.hpp"
#include "MStar.hpp"
#include "RedundancyRemoval.hpp"
#include "Scores.hpp"
#include "Solution.hpp"
#include "cft.hpp"

/**
 * @brief (It goes purposelly against RAII to maintain the memory allocations between subsequent calls)
 *
 */
class Greedy {
public:
    Greedy(SubInstance& subinst_, MStar& M_star_) : subinst(subinst_), M_star(M_star_), sigmas(subinst_), remove_redundat_cols(subinst_) { }

    /**
     * @brief Greedy procedure that finds feasible solutions starting from a set of Lagrangian multipliers.
     *
     * @param u_k   Array of Lagrangian multipliers
     * @param S     LocalSolution
     */
    LocalSolution operator()(const LocalMultipliers& u_k) {
        LocalSolution S;
        operator()(u_k, std::numeric_limits<idx_t>::max(), S);
        return S;
    }

    NO_INLINE void operator()(const LocalMultipliers& u_k, idx_t Ssize, LocalSolution& S) {

        M_star.reset_covered(subinst.get_cols(), S, subinst.get_nrows());

        // Score initialization
        sigmas.setup_scores(u_k, M_star);

        while (M_star.get_uncovered() > 0 && S.size() < Ssize) {
            idx_t j_star = sigmas.argmin_score();       // Step 2
            sigmas.update_scores(u_k, j_star, M_star);  // Step 2'

            assert(std::find(S.begin(), S.end(), j_star) == S.end());

            M_star.cover_rows(subinst.get_col(j_star));
            S.emplace_back(j_star);  // Step 3
        }

        IF_DEBUG {
            assert(M_star.get_covered() == subinst.get_nrows() || S.size() >= Ssize);
            M_star.reset_covered(subinst.get_cols(), S, subinst.get_nrows());
            if (S.size() < Ssize) { assert(M_star.get_uncovered() == 0); }
        }

        remove_redundat_cols(S, u_k, M_star);

        IF_DEBUG {
            M_star.reset_covered(subinst.get_cols(), S, subinst.get_nrows());
            if (S.size() < Ssize) { assert(M_star.get_uncovered() == 0); }
        }
    }

private:
    SubInstance& subinst;
    MStar& M_star;
    Scores sigmas;
    RendundacyRemoval remove_redundat_cols;
};


#endif