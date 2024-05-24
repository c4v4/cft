// SPDX-FileCopyrightText: 2024 Francesco Cavaliere <francescocava95@gmail.com>
// SPDX-License-Identifier: MIT

#ifndef CFT_SRC_CORE_SPARSEBINMAT_HPP
#define CFT_SRC_CORE_SPARSEBINMAT_HPP


#include <vector>

#include "utils/Span.hpp"
#include "utils/assert.hpp"  // IWYU pragma:  keep

namespace cft {

// A simple sparse binary matrix. Operator[] returns a span to the i-th element.
// For fine grained manipulations, idxs and begs are public.
template <typename IdxT>
struct SparseBinMat {

    std::vector<IdxT>   idxs;
    std::vector<size_t> begs = {0};

    Span<IdxT*> operator[](std::size_t i) {
        assert(i < begs.size() - 1U && begs[i + 1U] <= idxs.size());
        return make_span(idxs.data() + begs[i], idxs.data() + begs[i + 1]);
    }

    Span<IdxT const*> operator[](std::size_t i) const {
        assert(i < begs.size() - 1U && begs[i + 1U] <= idxs.size());
        return make_span(idxs.data() + begs[i], idxs.data() + begs[i + 1]);
    }

    std::size_t size() const {
        return begs.size() - 1ULL;
    }

    bool empty() const {
        return begs.size() == 1ULL;
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

    template <typename IdxT2>
    void push_back(std::initializer_list<IdxT2> elem) {
        idxs.insert(idxs.end(), elem.begin(), elem.end());
        begs.push_back(idxs.size());
    }
};


}  // namespace cft


#endif /* CFT_SRC_CORE_SPARSEBINMAT_HPP */
