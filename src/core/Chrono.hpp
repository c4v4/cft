// Copyright (c) 2022 Francesco Cavaliere
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

#ifndef CAV_SRC_CORE_CHRONO_HPP
#define CAV_SRC_CORE_CHRONO_HPP


#include <chrono>

#include "core/cft.hpp"

namespace cft {

using nsec    = std::chrono::nanoseconds;
using usec    = std::chrono::microseconds;
using msec    = std::chrono::milliseconds;
using sec     = std::chrono::seconds;
using minutes = std::chrono::minutes;
using hours   = std::chrono::hours;

template <typename UnitT = usec>
struct Chrono {
    using high_res_clock = std::chrono::steady_clock;
    using time_point     = std::chrono::steady_clock::time_point;

    time_point start;

    Chrono()
        : start(high_res_clock::now()) {
    }

    template <typename UnitT2>
    CFT_NODISCARD static constexpr double time_cast(double t) {
        using factor = std::ratio_divide<typename UnitT::period, typename UnitT2::period>;
        return t * factor::num / factor::den;
    }

    // Time elapsed since the start of the timer, resets the timer to the current istant
    CFT_NODISCARD uint64_t restart() {
        auto old = start;
        start    = high_res_clock::now();
        return std::chrono::duration_cast<UnitT>(start - old).count();
    }

    template <typename UnitT2>
    CFT_NODISCARD double restart() {
        return time_cast<UnitT2>(restart());
    }

    // Time elapsed since the start of the timer, does not reset the timer
    CFT_NODISCARD uint64_t elapsed() const {
        auto now = high_res_clock::now();
        return std::chrono::duration_cast<UnitT>(now - start).count();
    }

    template <typename UnitT2>
    CFT_NODISCARD double elapsed() const {
        return time_cast<UnitT2>(elapsed());
    }
};

}  // namespace cft

#endif /* CAV_SRC_CORE_CHRONO_HPP */