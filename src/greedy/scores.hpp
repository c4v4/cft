// SPDX-FileCopyrightText: 2024 Francesco Cavaliere <francescocava95@gmail.com>
// SPDX-License-Identifier: MIT

#ifndef CFT_SRC_GREEDY_SCORES_HPP
#define CFT_SRC_GREEDY_SCORES_HPP

#ifndef NDEBUG
#include <cmath>
#endif

#include "core/Instance.hpp"
#include "core/cft.hpp"
#include "utils/CoverCounters.hpp"
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

using score_subspan_t = Span<container_iterator_t<std::vector<ScoreData>>>;

struct Scores {
    std::vector<ScoreData> scores;        // column scores
    std::vector<real_t>    gammas;        // gamma values used to compute scores
    std::vector<ridx_t>    covered_rows;  // number of rows that can be covered by each column
    std::vector<cidx_t>    score_map;     // maps column index to score index
};

namespace local { namespace {

    // Score computed as described in the paper. Mu represents the number of rows that would be
    // covered by the current column. Gamma represents the reduced cost of the column minus the
    // component of the lagrangian multiplier associated with already covered rows.
    inline real_t compute_score(real_t gamma, ridx_t mu) {
        if (mu == 0_R)
            return limits<real_t>::max();
        if (gamma > 0.0_F)
            return gamma / as_real(mu);
        return gamma * as_real(mu);
    }

    template <typename Hook>
    void update_row_scores(std::vector<cidx_t> const& row,          // in
                           real_t                     i_lagr_mult,  // in
                           Scores&                    score_info,   // inout
                           Hook                       update_hook   // in
    ) {
        auto& scores = score_info.scores;  // shorthand

        for (cidx_t j : row) {
            score_info.covered_rows[j] -= 1_R;
            score_info.gammas[j] += i_lagr_mult;

            cidx_t s = score_info.score_map[j];
            assert(s != removed_cidx && "Column is not in the score map");
            scores[s].score = local::compute_score(score_info.gammas[j],
                                                   score_info.covered_rows[j]);
            assert(std::isfinite(score_info.gammas[j]) && "Gamma is not finite");
            assert(std::isfinite(scores[s].score) && "Score is not finite");
            update_hook(s);  // further ops on s element
        }
    }

}  // namespace
}  // namespace local

// Initializes the scores for the greedy algorithm.
inline void complete_scores_init(Instance const& inst,       // in
                                 Scores&         score_info  // inout
) {
    assert(csize(score_info.gammas) == csize(inst.cols) && "Expected initialized gammas vector");

    cidx_t ncols = csize(inst.cols);
    score_info.scores.clear();
    score_info.score_map.resize(ncols);
    score_info.covered_rows.resize(ncols);

    for (cidx_t j = 0_C; j < csize(inst.cols); ++j) {
        ridx_t cover_num           = rsize(inst.cols[j]);
        real_t score               = local::compute_score(score_info.gammas[j], cover_num);
        score_info.score_map[j]    = j;
        score_info.covered_rows[j] = cover_num;
        score_info.scores.push_back({score, j});
        assert(std::isfinite(score_info.gammas[j]) && "Gamma is not finite");
        assert(std::isfinite(score) && "Score is not finite");
    }
}

inline ridx_t update_covered(Instance const&            inst,         // in
                             std::vector<cidx_t> const& sol,          // in
                             std::vector<real_t> const& lagr_mult,    // in
                             Scores&                    score_info,   // inout
                             CoverCounters&             row_coverage  // inout
) {
    ridx_t covered_rows = 0_R;
    for (cidx_t j : sol)
        covered_rows += as_ridx(row_coverage.cover(inst.cols[j]));

    for (ridx_t i = 0_R; i < rsize(row_coverage); i++)
        if (row_coverage[i] > 0)
            local::update_row_scores(inst.rows[i], lagr_mult[i], score_info, NoOp{});

    return covered_rows;
}

template <typename Hook>
inline void update_changed_scores(Instance const&            inst,          // in
                                  std::vector<real_t> const& lagr_mult,     // in
                                  CoverCounters const&       row_coverage,  // in
                                  cidx_t                     jstar,         // in
                                  Scores&                    score_info,    // inout
                                  Hook                       update_hook    // in
) {
    auto col_star = inst.cols[jstar];
    for (ridx_t i : col_star)
        if (row_coverage[i] == 0)
            local::update_row_scores(inst.rows[i], lagr_mult[i], score_info, update_hook);
}

inline score_subspan_t select_good_scores(Scores& score_info,  // in
                                          cidx_t  how_many     // in
) {
    assert(how_many > 0_C && "Good size must be greater than 0");
    auto& scores = score_info.scores;  // shorthand

    how_many = std::min(how_many, csize(scores));
    cft::nth_element(scores, how_many - 1_C, ScoreKey{});
    for (cidx_t s = 0_C; s < csize(scores); ++s)
        score_info.score_map[scores[s].idx] = s;

    return make_span(scores.begin(), how_many);
}
}  // namespace cft


#endif /* CFT_SRC_GREEDY_SCORES_HPP */
