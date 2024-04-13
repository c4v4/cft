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

#ifndef CFT_SRC_CORE_SPARSEBINMAT_HPP
#define CFT_SRC_CORE_SPARSEBINMAT_HPP


#include <vector>

#include "utils/Span.hpp"
#include "utils/assert.hpp" // IWYU pragma:  keep

namespace cft {

// A simple sparse binary matrix. Operator[] returns a span to the i-th element (row or column).
// For finer control, idxs and begs are public.
template <typename IdxT>
struct SparseBinMat {

    std::vector<IdxT>   idxs;
    std::vector<size_t> begs = {0};

    Span<IdxT*> operator[](std::size_t i) {
        assert(i < begs.size() - 1);
        assert(begs[i] < idxs.size());
        assert(begs[i + 1] <= idxs.size());
        return {idxs.data() + begs[i], idxs.data() + begs[i + 1]};
    }

    Span<IdxT const*> operator[](std::size_t i) const {
        assert(i < begs.size());
        assert(begs[i] < idxs.size());
        assert(begs[i + 1] <= idxs.size());
        return {idxs.data() + begs[i], idxs.data() + begs[i + 1]};
    }

    std::size_t size() const {
        return begs.size() - 1;
    }

    bool empty() const {
        return begs.size() == 1;
    }

    void clear() {
        idxs.clear();
        begs.clear();
        begs.push_back(0);
    }

    template <typename T>
    void push_back(T const& elem) {
        idxs.insert(idxs.end(), elem.begin(), elem.end());
        begs.push_back(idxs.size());
    }

    void push_back(std::initializer_list<IdxT> elem) {
        idxs.insert(idxs.end(), elem.begin(), elem.end());
        begs.push_back(idxs.size());
    }
};


}  // namespace cft


#endif /* CFT_SRC_CORE_SPARSEBINMAT_HPP */
