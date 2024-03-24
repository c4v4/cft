#ifndef CFT_SRC_CORE_GREEDYSCORES_HPP
#define CFT_SRC_CORE_GREEDYSCORES_HPP

#include <cmath>

#include "core/cft.hpp"
#include "core/coverage.hpp"
#include "core/limits.hpp"
#include "core/sort.hpp"
#include "instance/Instance.hpp"

namespace cft {
struct ScoreData {
    real_t score;
    cidx_t idx;
};

struct ScoreKey {
    CFT_NODISCARD real_t operator()(ScoreData sd) const {
        return sd.score;
    }
};

using score_subspan_t = Span<std::vector<ScoreData>::iterator>;

struct Scores {
    std::vector<ScoreData> scores;        // column scores
    std::vector<real_t>    gammas;        // gamma values used to compute scores
    std::vector<ridx_t>    covered_rows;  // number of rows that can be covered by each column
    std::vector<cidx_t>    score_map;     // maps column index to score index
};

// Score computed as descrived in the paper. Mu represents the number of still uncovered rows that
// would be covered by the current column. Gamma represents the reduced cost of the column minus the
// component of the lagrangian multiplier associated with already covered rows.
CFT_NODISCARD inline real_t compute_score(real_t gamma, ridx_t mu) {
    if (mu == 0)
        return limits<real_t>::max();
    if (gamma > 0)
        return gamma / static_cast<real_t>(mu);
    return gamma * static_cast<real_t>(mu);
}

// Initialize the scores for the greedy algorithm
inline void complete_scores_init(Instance const& inst, Scores& score_info) {
    assert(score_info.gammas.size() == inst.cols.size() && "Expected initialized gammas vector");

    cidx_t ncols = inst.cols.size();
    score_info.scores.clear();
    score_info.score_map.resize(ncols);
    score_info.covered_rows.resize(ncols);

    for (cidx_t j = 0; j < inst.cols.size(); ++j) {
        cidx_t cover_num           = inst.cols[j].size();
        real_t score               = compute_score(score_info.gammas[j], cover_num);
        score_info.score_map[j]    = j;
        score_info.covered_rows[j] = cover_num;
        score_info.scores.push_back({score, j});
        assert(std::isfinite(score_info.gammas[j]) && "Gamma is not finite");
        assert(std::isfinite(score) && "Score is not finite");
    }
}

inline void update_row_scores(std::vector<cidx_t> const& row,
                              real_t                     i_lagr_mult,
                              Scores&                    score_info) {
    auto& scores = score_info.scores;  // shorthand

    for (cidx_t j : row) {
        score_info.covered_rows[j] -= 1;
        score_info.gammas[j] += i_lagr_mult;

        cidx_t s = score_info.score_map[j];
        assert(s != CFT_REMOVED_IDX && "Column is not in the score map");
        scores[s].score = compute_score(score_info.gammas[j], score_info.covered_rows[j]);
        assert(std::isfinite(score_info.gammas[j]) && "Gamma is not finite");
        assert(std::isfinite(scores[s].score) && "Score is not finite");
    }
}

CFT_NODISCARD inline cidx_t update_already_covered(Instance const&            inst,
                                                   Solution const&            sol,
                                                   std::vector<real_t> const& lagr_mult,
                                                   Scores&                    score_info,
                                                   CoverCounters<>&           total_cover) {

    cidx_t covered_rows = 0;
    for (cidx_t j : sol.idxs)
        covered_rows += total_cover.cover(inst.cols[j]);

    for (ridx_t i = 0; i < total_cover.size(); i++)
        if (total_cover[i] > 0)
            update_row_scores(inst.rows[i], lagr_mult[i], score_info);

    return covered_rows;
}

inline void update_changed_scores(Instance const&            inst,
                                  std::vector<real_t> const& lagr_mult,
                                  CoverCounters<> const&     total_cover,
                                  cidx_t                     score_argmin,
                                  Scores&                    score_info) {
    cidx_t jstar    = score_info.scores[score_argmin].idx;
    auto   col_star = inst.cols[jstar];

    for (ridx_t i : col_star)
        if (total_cover[i] == 0)
            update_row_scores(inst.rows[i], lagr_mult[i], score_info);
}

inline score_subspan_t compute_good_scores(Sorter& sorter, Scores& score_info, size_t good_size) {
    assert(good_size > 0 && "Good size must be greater than 0");
    auto& scores = score_info.scores;  // shorthand

    remove_if(scores, [&](ScoreData sd) {
        if (sd.score < limits<real_t>::max())
            return false;
        score_info.score_map[sd.idx] = CFT_REMOVED_IDX;
        return true;
    });

    good_size = std::min(good_size, scores.size());
    sorter.nth_element(scores, good_size, ScoreKey{});
    for (cidx_t s = 0; s < scores.size(); ++s)
        score_info.score_map[scores[s].idx] = s;

    return make_span(scores.begin(), good_size);
}


}  // namespace cft

#endif /* CFT_SRC_CORE_GREEDYSCORES_HPP */
