#ifndef CFT_SRC_CORE_GREEDYSCORES_HPP
#define CFT_SRC_CORE_GREEDYSCORES_HPP

#include "core/cft.hpp"
#include "core/coverage.hpp"
#include "core/limits.hpp"
#include "core/sort.hpp"
#include "instance/Instance.hpp"

namespace cft {

namespace {
    struct ScoreData {
        real_t gamma;
        real_t score;
        cidx_t col_idx;
        ridx_t cover_count;
    };
}  // namespace

struct Scores {
private:
    std::vector<ScoreData> scores;
    std::vector<cidx_t>    score_map;  // maps column index to score index
    cidx_t                 good_size = 0;

    static inline real_t _compute_score(real_t gamma, ridx_t mu) {
        if (mu == 0 || mu == REMOVED_INDEX)
            return limits<real_t>::max();
        if (gamma > 0)
            return gamma / static_cast<real_t>(mu);
        return gamma * static_cast<real_t>(mu);
    }

    inline void _init_empty_sol_scores(Instance const& inst, std::vector<real_t> const& lagr_mult) {
        for (cidx_t j = 0; j < inst.cols.size(); ++j) {
            real_t gamma = inst.costs[j];
            for (ridx_t i : inst.cols[j])
                gamma -= lagr_mult[i];
            cidx_t cover_num = inst.cols[j].size();
            real_t score     = _compute_score(gamma, cover_num);
            scores.push_back({gamma, score, j, cover_num});
        }
    }

    inline void _init_partial_sol_scores(Instance const&            inst,
                                         std::vector<real_t> const& lagr_mult,
                                         CoverBits const&           cover_bits) {
        for (cidx_t j = 0; j < inst.cols.size(); ++j) {
            ridx_t cover_count = 0;
            real_t gamma       = inst.costs[j];

            for (ridx_t i : inst.cols[j])
                if (!cover_bits[i]) {
                    ++cover_count;
                    gamma -= lagr_mult[i];
                }
            real_t score = _compute_score(gamma, cover_count);
            scores.push_back({gamma, score, j, cover_count});
        }
    }

    void _update_scores(Instance const&            inst,
                        std::vector<real_t> const& lagr_mult,
                        CoverBits const&           cover_bits,
                        cidx_t                     score_argmin) {

        cidx_t jstar                 = scores[score_argmin].col_idx;
        scores[score_argmin].col_idx = REMOVED_INDEX;

        auto col_star = inst.cols[jstar];
        for (ridx_t i : col_star) {
            if (cover_bits[i])
                continue;
            for (cidx_t j : inst.rows[i]) {
                auto& score_data = scores[score_map[j]];
                if (score_data.col_idx == REMOVED_INDEX)
                    continue;
                score_data.cover_count -= 1;
                score_data.gamma += lagr_mult[i];
                score_data.score = _compute_score(score_data.gamma, score_data.cover_count);
            }
        }
    }

public:
    inline void init_scores(Instance const&            inst,
                            std::vector<real_t> const& lagr_mult,
                            CoverBits const&           cover_bits,
                            cidx_t                     sol_size,
                            Sorter&                    sorter) {

        ridx_t nrows = inst.rows.size();
        cidx_t ncols = inst.cols.size();

        scores.clear();
        if (sol_size == 0)
            _init_empty_sol_scores(inst, lagr_mult);
        else
            _init_partial_sol_scores(inst, lagr_mult, cover_bits);

        good_size = min(nrows, ncols - sol_size);
        sorter.nth_element(scores, good_size, [](ScoreData sd) { return sd.score; });

        score_map.resize(ncols);
        for (cidx_t s = 0; s < scores.size(); ++s)
            score_map[scores[s].col_idx] = s;
    }

    inline cidx_t extract_minscore_col(Instance const&            inst,
                                       std::vector<real_t> const& lagr_mult,
                                       CoverBits&                 cover_bits,
                                       Sorter&                    sorter) {

        auto   good_scores  = make_span(scores.begin(), good_size);
        cidx_t score_argmin = argmin(good_scores, [](ScoreData sd) { return sd.score; });
        real_t score_min    = good_scores[score_argmin].score;

        if (score_min > scores[good_size].score) {
            sorter.nth_element(scores, good_size, [](ScoreData sd) { return sd.score; });
            for (cidx_t s = 0; s < scores.size(); ++s)
                score_map[scores[s].col_idx] = s;
            score_argmin = argmin(good_scores, [](ScoreData sd) { return sd.score; });
        }

        _update_scores(inst, lagr_mult, cover_bits, score_argmin);
        return scores[score_argmin].col_idx;
    }
};

inline Scores make_greedy_scores() {
    return Scores{};
}
}  // namespace cft

#endif /* CFT_SRC_CORE_GREEDYSCORES_HPP */
