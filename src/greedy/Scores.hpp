#ifndef CFT_SRC_CORE_GREEDYSCORES_HPP
#define CFT_SRC_CORE_GREEDYSCORES_HPP

#include "core/cft.hpp"
#include "core/coverage.hpp"
#include "core/limits.hpp"
#include "core/sort.hpp"
#include "instance/Instance.hpp"

namespace cft {
struct ScoreData {
    real_t gamma;
    real_t score;
    cidx_t col_idx;
    ridx_t cover_count;
};

struct ScoreFtor {
    CFT_NODISCARD real_t operator()(ScoreData sd) const {
        return sd.score;
    }
};

struct Scores {
private:
    std::vector<ScoreData> scores;     // column scores
    std::vector<cidx_t>    score_map;  // maps column index to score index
    cidx_t good_size         = 0;      // only a subset of the scores are considered (good-columns)
    cidx_t active_cols       = 0;      // number of non redundant columns
    real_t good_thresh_score = limits<real_t>::max();  // worst non-good column score

    static real_t _compute_score(real_t gamma, ridx_t mu) {
        if (mu == 0)
            return limits<real_t>::max();
        if (gamma > 0)
            return gamma / static_cast<real_t>(mu);
        return gamma * static_cast<real_t>(mu);
    }

    void _update_changed_scores(Instance const&            inst,
                                std::vector<real_t> const& lagr_mult,
                                CoverCounters<> const&     cover_counts,
                                cidx_t                     score_argmin) {

        cidx_t jstar    = scores[score_argmin].col_idx;
        auto   col_star = inst.cols[jstar];
        for (ridx_t i : col_star) {
            if (cover_counts[i] > 0)
                continue;
            for (cidx_t j : inst.rows[i]) {
                auto& score_data = scores[score_map[j]];
                score_data.cover_count -= 1;
                score_data.gamma += lagr_mult[i];

                score_data.score = _compute_score(score_data.gamma, score_data.cover_count);
                assert(std::isfinite(score_data.gamma) && "Gamma is not finite");
                assert(std::isfinite(score_data.score) && "Score is not finite");
                if (score_data.cover_count == 0)
                    --active_cols;
            }
        }
    }

    Span<std::vector<ScoreData>::iterator> _update_good_scores(Sorter& sorter) {
        good_size = min(good_size, active_cols);
        sorter.nth_element(scores, good_size, ScoreFtor{});
        good_thresh_score = scores[good_size - 1].score;
        for (cidx_t s = 0; s < scores.size(); ++s)
            score_map[scores[s].col_idx] = s;
        return make_span(scores.begin(), good_size);
    }

public:
    // Initialize the scores for the greedy algorithm (starting from an empty solution)
    void init_scores(Instance const& inst, std::vector<real_t> const& lagr_mult, Sorter& sorter) {

        ridx_t nrows = inst.rows.size();
        cidx_t ncols = inst.cols.size();
        scores.clear();
        score_map.resize(ncols);

        active_cols = inst.cols.size();
        for (cidx_t j = 0; j < inst.cols.size(); ++j) {
            real_t gamma = inst.costs[j];
            for (ridx_t i : inst.cols[j]) {
                gamma -= lagr_mult[i];
                assert(std::isfinite(lagr_mult[i]) && "Multiplier is not finite");
            }

            cidx_t cover_num = inst.cols[j].size();
            real_t score     = _compute_score(gamma, cover_num);
            scores.push_back({gamma, score, j, cover_num});
            assert(std::isfinite(gamma) && "Gamma is not finite");
            assert(std::isfinite(score) && "Score is not finite");
        }

        good_size = min(nrows, active_cols);
        _update_good_scores(sorter);
    }

    // Initialize the scores for the greedy algorithm (starting from an partially filled
    // solution)
    void init_scores(Instance const&            inst,
                     std::vector<real_t> const& lagr_mult,
                     CoverCounters<> const&     cover_counts,
                     Sorter&                    sorter) {

        ridx_t nrows = inst.rows.size();
        cidx_t ncols = inst.cols.size();
        scores.clear();
        score_map.resize(ncols);

        active_cols = inst.cols.size();
        for (cidx_t j = 0; j < inst.cols.size(); ++j) {
            ridx_t cover_count = 0;
            real_t gamma       = inst.costs[j];

            for (ridx_t i : inst.cols[j])
                if (cover_counts[i] == 0) {
                    ++cover_count;
                    gamma -= lagr_mult[i];
                    assert(std::isfinite(lagr_mult[i]) && "Multiplier is not finite");
                }
            if (cover_count == 0)
                --active_cols;
            real_t score = _compute_score(gamma, cover_count);
            scores.push_back({gamma, score, j, cover_count});
            assert(std::isfinite(gamma) && "Gamma is not finite");
            assert(std::isfinite(score) && "Score is not finite");
        }

        good_size = min(nrows, active_cols);
        _update_good_scores(sorter);
    }

    // Extract the column with the minimum score and update the scores
    cidx_t extract_minscore_col(Instance const&            inst,
                                std::vector<real_t> const& lagr_mult,
                                CoverCounters<> const&     cover_counts,
                                Sorter&                    sorter) {

        auto   good_scores  = make_span(scores.begin(), good_size);
        cidx_t score_argmin = argmin(good_scores, ScoreFtor{});
        real_t score_min    = good_scores[score_argmin].score;

        if (score_min > good_thresh_score) {
            good_scores  = _update_good_scores(sorter);
            score_argmin = argmin(good_scores, ScoreFtor{});
        }

        _update_changed_scores(inst, lagr_mult, cover_counts, score_argmin);
        return scores[score_argmin].col_idx;
    }
};

}  // namespace cft

#endif /* CFT_SRC_CORE_GREEDYSCORES_HPP */
