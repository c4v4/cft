// SPDX-FileCopyrightText: 2024 Francesco Cavaliere <francescocava95@gmail.com>
// SPDX-License-Identifier: MIT

#ifndef CFT_SRC_CORE_CHRONO_HPP
#define CFT_SRC_CORE_CHRONO_HPP


#include <chrono>

#include "utils/utility.hpp"

namespace cft {

using nsec    = std::chrono::nanoseconds;
using usec    = std::chrono::microseconds;
using msec    = std::chrono::milliseconds;
using sec     = std::chrono::seconds;
using minutes = std::chrono::minutes;
using hours   = std::chrono::hours;

// Chrono class to measure time elapsed since the start of the timer
template <typename UnitT = usec>
struct Chrono {
    using high_res_clock = std::chrono::steady_clock;
    using time_point     = std::chrono::steady_clock::time_point;

    time_point start;

    Chrono()
        : start(high_res_clock::now()) {
    }

    template <typename UnitT2>
    static constexpr double time_cast(double t) {
        using factor = std::ratio_divide<typename UnitT::period, typename UnitT2::period>;
        return t * factor::num / factor::den;
    }

    // Time elapsed since the start of the timer, resets the timer to the current istant
    int64_t restart() {
        auto old = start;
        start    = high_res_clock::now();
        return std::chrono::duration_cast<UnitT>(start - old).count();
    }

    template <typename UnitT2>
    double restart() {
        return time_cast<UnitT2>(restart());
    }

    // Time elapsed since the start of the timer, does not reset the timer
    int64_t elapsed() const {
        auto now = high_res_clock::now();
        return std::chrono::duration_cast<UnitT>(now - start).count();
    }

    template <typename UnitT2>
    double elapsed() const {
        return time_cast<UnitT2>(checked_cast<double>(elapsed()));
    }
};

}  // namespace cft


#endif /* CFT_SRC_CORE_CHRONO_HPP */
