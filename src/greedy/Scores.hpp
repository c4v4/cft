// Copyright (c) 2024 Francesco Cavaliere
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>.

#ifndef CFT_SRC_GREEDY_SCORES_HPP
#define CFT_SRC_GREEDY_SCORES_HPP


#include <cmath>

#include "core/Instance.hpp"
#include "core/cft.hpp"
#include "utils/coverage.hpp"
#include "utils/limits.hpp"
#include "utils/sort.hpp"
#include "utils/utility.hpp"

namespace cft {
struct ScoreData {
    real_t score;
    cidx_t idx;
};

struct ScoreKey {
    real_t operator()(ScoreData sd) const {
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

// Score computed as descrived in the paper. Mu represents the number of still uncovered rows
// that would be covered by the current column. Gamma represents the reduced cost of the column
// minus the component of the lagrangian multiplier associated with already covered rows.
inline real_t compute_score(real_t gamma, ridx_t mu) {
    if (mu == 0_R)
        return limits<real_t>::max();
    if (gamma > 0.0_F)
        return gamma / as_real(mu);
    return gamma * as_real(mu);
}

// Initialize the scores for the greedy algorithm
inline void complete_scores_init(Instance const& inst, Scores& score_info) {
    assert(csize(score_info.gammas) == csize(inst.cols) && "Expected initialized gammas "
                                                           "vector");

    cidx_t ncols = csize(inst.cols);
    score_info.scores.clear();
    score_info.score_map.resize(ncols);
    score_info.covered_rows.resize(ncols);

    for (cidx_t j = 0_C; j < csize(inst.cols); ++j) {
        ridx_t cover_num           = rsize(inst.cols[j]);
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
        assert(s != removed_cidx && "Column is not in the score map");
        scores[s].score = compute_score(score_info.gammas[j], score_info.covered_rows[j]);
        assert(std::isfinite(score_info.gammas[j]) && "Gamma is not finite");
        assert(std::isfinite(scores[s].score) && "Score is not finite");
    }
}

inline ridx_t update_covered(Instance const&            inst,
                             Solution const&            sol,
                             std::vector<real_t> const& lagr_mult,
                             Scores&                    score_info,
                             CoverCounters<>&           total_cover) {

    ridx_t covered_rows = 0_R;
    for (cidx_t j : sol.idxs)
        covered_rows += as_ridx(total_cover.cover(inst.cols[j]));

    for (ridx_t i = 0_R; i < rsize(total_cover); i++)
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

inline score_subspan_t get_good_scores(Scores& score_info, size_t amount) {
    assert(amount > 0 && "Good size must be greater than 0");
    auto& scores = score_info.scores;  // shorthand

    remove_if(scores, [&](ScoreData sd) {
        if (sd.score < limits<real_t>::max())
            return false;
        score_info.score_map[sd.idx] = removed_cidx;
        return true;
    });

    amount = std::min(amount, scores.size());
    cft::nth_element(scores, amount, ScoreKey{});
    for (cidx_t s = 0_C; s < csize(scores); ++s)
        score_info.score_map[scores[s].idx] = s;

    return make_span(scores.begin(), amount);
}
}  // namespace cft


#endif /* CFT_SRC_GREEDY_SCORES_HPP */
