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

#ifndef CFT_SRC_CORE_GREEDYENUMERATION_HPP
#define CFT_SRC_CORE_GREEDYENUMERATION_HPP

#include "core/cft.hpp"
#include "greedy/RedundancySet.hpp"

#define ENUM_VARS 10

namespace cft {

template <size_t Cur>
struct Enumerator;

template <>
struct Enumerator<ENUM_VARS> {
    static void invoke(RedundancyData& state, real_t lb, bool const* vars, bool* sol) {
        if (lb < state.ub) {
            state.ub = lb;
            for (cidx_t s = 0; s < ENUM_VARS; ++s)
                sol[s] = vars[s];
        }
    }
};

template <size_t Cur>
struct Enumerator {
    static void invoke(RedundancyData& red_data, real_t lb, bool* vars, bool* sol) {
        if (Cur == red_data.redund_set.size()) {
            Enumerator<ENUM_VARS>::invoke(red_data, lb, vars, sol);
            return;
        }

        auto col    = red_data.redund_set[Cur].col;
        auto new_lb = lb + red_data.redund_set[Cur].cost;
        if (new_lb < red_data.ub && !red_data.cover_bits.is_redundant_cover(col)) {
            vars[Cur] = true;
            red_data.cover_bits.cover(col);
            Enumerator<Cur + 1>::invoke(red_data, new_lb, vars, sol);
            vars[Cur] = false;
            red_data.cover_bits.uncover(col);
        }

        if (red_data.cover_counts.is_redundant_uncover(col))
            Enumerator<Cur + 1>::invoke(red_data, lb, vars, sol);
    }
};

}  // namespace cft

#endif /* CFT_SRC_CORE_GREEDYENUMERATION_HPP */
