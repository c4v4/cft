// Copyright (c) 2024 Luca Accorsi and Francesco Cavaliere
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

#ifndef CFT_SRC_ALGORITHMS_REFINMENT_HPP
#define CFT_SRC_ALGORITHMS_REFINMENT_HPP

#include "algorithms/ThreePhase.hpp"
#include "core/cft.hpp"

namespace cft {

// Complete CFT algorithm (Refinement + call to 3-phase)
inline Solution run(Instance& inst, prng_t& rnd) {
    // TODO(any): implement the `refinement` procedure
    ThreePhase three_phase;
    return three_phase(inst, rnd);
}

}  // namespace cft

#endif /* CFT_SRC_ALGORITHMS_REFINMENT_HPP */
