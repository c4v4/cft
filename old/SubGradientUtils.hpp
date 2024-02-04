#ifndef SCP_INCLUDE_SUBGRADIENTUTILS_HPP_
#define SCP_INCLUDE_SUBGRADIENTUTILS_HPP_

#include "Counter.hpp"
#include "Instance.hpp"
#include "cft.hpp"
class StepSizeFactor {
public:
    StepSizeFactor(real_t initial_value_ = 0.1, idx_t period = 20) : p(period), lambda(initial_value_),
                                                                     wrst_LB(REAL_MAX), best_LB(REAL_LOWEST) { }

    inline void update(real_t current_LB) {
        if (current_LB < wrst_LB) {
            wrst_LB = current_LB;
        }
        if (current_LB > best_LB) {
            best_LB = current_LB;
        }

        p.inc();
        if (p.reached()) {
            const auto perc_diff = (best_LB - wrst_LB) / best_LB;
            //assert(perc_diff > 0.0);

            if (perc_diff > 0.01) {
                lambda /= 2.0;
            } else if (perc_diff <= 0.001) {
                lambda *= 1.5;
            }

            p.restart();
            best_LB = REAL_LOWEST;
            wrst_LB = REAL_MAX;
        }
    }

    [[nodiscard]] inline real_t get() const { return lambda; }

    inline void reset() {
        p.restart();
        best_LB = REAL_LOWEST;
        wrst_LB = REAL_MAX;
    }

private:
    Counter p;
    real_t lambda;
    real_t wrst_LB;
    real_t best_LB;
};

class PricingPeriod : public Counter {
public:
    PricingPeriod(idx_t period = 10U, idx_t period_lim = 1000UL) : Counter(period), Tlim(period_lim) { }

    void restart() = delete;

    inline void reset(real_t global_LB, real_t local_LB, real_t UB) {
        Counter::restart();

        const auto delta = (local_LB - global_LB) / UB;

        if (delta <= 0.000001) {
            set_max(std::min<idx_t>(Tlim, get_max() * 10));
        } else if (delta <= 0.02) {
            set_max(std::min<idx_t>(Tlim, get_max() * 5));
        } else if (delta <= 0.2) {
            set_max(std::min<idx_t>(Tlim, get_max() * 2));
        } else {
            set_max(10);
        }
    }

private:
    idx_t Tlim;
};

class ExitCondition {
public:
    explicit ExitCondition(idx_t period_ = 300) : period(period_), counter(period_), LB_past(REAL_LOWEST) { }

    bool operator()(real_t LB) {
        if (--counter == 0) {
            const auto impr = LB - LB_past;
            const auto gap = 2.0 * (LB - LB_past) / LB;

            LB_past = LB;
            counter = period;

            return impr < 1 && gap < 0.001;
        }
        return false;
    }

private:
    const idx_t period;
    idx_t counter;
    real_t LB_past;
};

#endif