#ifndef SCP_INCLUDE_SCORE_HPP
#define SCP_INCLUDE_SCORE_HPP

#include <numeric>

#include "CountSet.hpp"
#include "Instance.hpp"
#include "Multipliers.hpp"
#include "cft.hpp"

class ScoreData {
public:
    ScoreData()
        : gamma(REAL_MAX)
        , mu(REMOVED_INDEX)
        , sigma(REAL_MAX) {
    }

    ScoreData(real_t gamma_, idx_t mu_)
        : gamma(gamma_)
        , mu(mu_)
        , sigma(_score(gamma_, mu_)) {
    }

    ScoreData(std::vector<real_t> const& u_k, BitSet const& covering_times, SubInstCol& j_col)
        : gamma(j_col.get_cost())
        , mu(0UL) {

        for (auto const i : j_col) {
            if (!covering_times[i]) {
                ++mu;
                gamma -= u_k[i];
            }
        }
        sigma = _score(gamma, mu);
    }

    void update(real_t u_k) {
        gamma += u_k;
        assert(mu > 0);
        --mu;
        sigma = _score(gamma, mu);
    }

    void inserted_in_S() {
        sigma = gamma = REAL_MAX;
        mu            = REMOVED_INDEX;
    }

    [[nodiscard]] bool is_marked() const {
        return mu == REMOVED_INDEX;
    }

    [[nodiscard]] real_t get_score() const {
        return sigma;
    }

private:
    static inline real_t _score(real_t gamma, idx_t mu) {
        if (mu == 0 || mu == REMOVED_INDEX)
            return REAL_MAX;
        if (gamma > 0)
            return gamma / static_cast<real_t>(mu);
        return gamma * static_cast<real_t>(mu);
    }

private:
    real_t gamma;
    idx_t  mu;
    real_t sigma;
};

class Scores {
public:
    explicit Scores(SubInstance& subinst_)
        : subinst(subinst_)
        , gam_mu_sig() {
    }

    // Setup for step 2
    inline void setup_scores(LocalMultipliers const& u_k, BitSet& covering_times) {

        auto& cols = subinst.get_cols();
        Bsize      = cols.size();
        B.resize(Bsize);
        std::iota(B.begin(), B.end(), 0);
        Ssize = 0;

        gam_mu_sig.resize(cols.size());
        for (idx_t j = 0; auto& col : cols) {
            gam_mu_sig[j] = ScoreData(u_k, covering_times, col);
            ++j;
        }

        tau = _compute_B_and_tau(std::min<idx_t>(subinst.get_nrows(), cols.size()));
    }

    // Step 1
    inline idx_t argmin_score() {

        idx_t bj = 0;
        while (bj < Bsize && gam_mu_sig[B[bj]].is_marked())
            ++bj;
        assert(bj < Bsize);

        idx_t  j_star      = B[bj];
        real_t sigma_jstar = gam_mu_sig[j_star].get_score();

        for (++bj; bj < Bsize; ++bj) {
            idx_t  j     = B[bj];
            real_t sigma = gam_mu_sig[j].get_score();
            if (sigma < sigma_jstar) {
                assert(!gam_mu_sig[j].is_marked());
                j_star      = j;
                sigma_jstar = sigma;
            }
        }

        if (sigma_jstar > tau) {
            tau = _compute_B_and_tau(
                std::min<idx_t>(subinst.get_nrows(), subinst.get_ncols() - Ssize));
            return argmin_score();
        }

        assert(!gam_mu_sig[j_star].is_marked());
        return j_star;
    }

    // Step 2'
    inline void update_scores(LocalMultipliers const& u_k,
                              const idx_t             j_star,
                              BitSet const&           covering_times) {
        assert(!gam_mu_sig[j_star].is_marked());
        gam_mu_sig[j_star].inserted_in_S();
        ++Ssize;

        auto& rows       = subinst.get_rows();
        auto& j_star_col = subinst.get_col(j_star);
        for (auto i : j_star_col) {
            if (!covering_times[i]) {
                for (auto j : rows[i])
                    if (!gam_mu_sig[j].is_marked())
                        gam_mu_sig[j].update(u_k[i]);
            }
        }
    }


private:
    inline real_t _compute_B_and_tau(idx_t size) {
        auto last = size - 1;
        assert(last < B.size());
        Bsize = size;
        std::nth_element(B.begin(), B.begin() + last, B.end(), [&](auto const a, auto const b) {
            return gam_mu_sig[a].get_score() < gam_mu_sig[b].get_score();
        });
        return gam_mu_sig[B[last]].get_score();
    }


private:
    SubInstance&           subinst;
    std::vector<ScoreData> gam_mu_sig;
    idx_t                  Ssize;
    idx_t                  Bsize;
    std::vector<idx_t>     B;
    real_t                 tau;
};


#endif
