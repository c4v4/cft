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
#include "instance/Instance.hpp"

#define CFT_ENUM_VARS 10

namespace cft {

/// @brief When the threshold is below a certain value, we can enumerate all possible non-redundant
/// combinations. As the threshold is known at compile-time, the enumeration has a fixed maximum
/// depth. Enumerator utilizes partial specialization (since `if constexpr` is a C++17 feature)
/// to limit the number of enumeration steps and (technically) remove the recursion.
template <size_t Cur>
struct Enumerator;

template <>
struct Enumerator<CFT_ENUM_VARS> {
    static void invoke(Instance const& /*inst*/,
                       RedundancyData& red_data,
                       bool const*     vars,
                       bool*           sol) {
        if (red_data.partial_cost < red_data.best_cost) {
            red_data.best_cost = red_data.partial_cost;
            for (cidx_t s = 0; s < CFT_ENUM_VARS; ++s)
                sol[s] = vars[s];
        }

#ifndef NDEBUG
        for (ridx_t i = 0; i < red_data.partial_cover.size(); ++i) {
            assert(red_data.total_cover[i] > 0);
            assert(red_data.partial_cover[i] == true);
        }
#endif
    }
};

template <size_t Depth>
struct Enumerator {
    static void invoke(Instance const& inst, RedundancyData& red_data, bool* vars, bool* sol) {

#ifndef NDEBUG
        for (ridx_t i = 0; i < red_data.partial_cover.size(); ++i)
            assert(red_data.total_cover[i] > 0);
#endif

        if (Depth == red_data.redund_set.size()) {
            Enumerator<CFT_ENUM_VARS>::invoke(inst, red_data, vars, sol);
            return;
        }

        auto col_idx = red_data.redund_set[Depth].col;

        assert(!red_data.partial_cover.is_redundant_cover(inst.cols[col_idx]) ||
               red_data.total_cover.is_redundant_uncover(inst.cols[col_idx]));

        if (red_data.partial_cost + red_data.redund_set[Depth].cost < red_data.best_cost &&
            !red_data.partial_cover.is_redundant_cover(inst.cols[col_idx])) {

            vars[Depth] = true;
            red_data.partial_cover.cover(inst.cols[col_idx]);
            red_data.partial_cost += red_data.redund_set[Depth].cost;

            Enumerator<Depth + 1>::invoke(inst, red_data, vars, sol);

            vars[Depth] = false;
            red_data.partial_cover.uncover(inst.cols[col_idx]);
            red_data.partial_cost -= red_data.redund_set[Depth].cost;
        }

        if (red_data.total_cover.is_redundant_uncover(inst.cols[col_idx])) {
            red_data.total_cover.uncover(inst.cols[col_idx]);
            Enumerator<Depth + 1>::invoke(inst, red_data, vars, sol);
            red_data.total_cover.cover(inst.cols[col_idx]);
        }
    }
};

}  // namespace cft

#endif /* CFT_SRC_CORE_GREEDYENUMERATION_HPP */
