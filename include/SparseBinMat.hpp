#ifndef CFT_INCLUDE_SPARSEBINMAT_HPP
#define CFT_INCLUDE_SPARSEBINMAT_HPP

#include <cassert>

#include "Span.hpp"
#include "cft.hpp"

namespace cft {

/// @brief A simple sparse binary matrix.
/// Operator[] returns a span to the i-th element (row or column).
/// For finer control, idxs and begs are public.
template <typename IdxT>
struct SparseBinMat {

    std::vector<IdxT>   idxs;
    std::vector<size_t> begs;

    CFT_NODISCARD Span<IdxT*> operator[](std::size_t i) {
        assert(i < begs.size() - 1);
        assert(begs[i] < idxs.size());
        assert(begs[i + 1] < idxs.size());
        return {idxs.data() + begs[i], idxs.data() + begs[i + 1]};
    }

    CFT_NODISCARD Span<IdxT const*> operator[](std::size_t i) const {
        assert(i < begs.size());
        assert(begs[i] < idxs.size());
        assert(begs[i + 1] < idxs.size());
        return {idxs.data() + begs[i], idxs.data() + begs[i + 1]};
    }

    CFT_NODISCARD std::size_t size() const {
        return begs.size() - 1;
    }

    CFT_NODISCARD bool empty() const {
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
};

template <typename IdxT>
CFT_NODISCARD inline SparseBinMat<IdxT> make_sparse_bin_mat() {
    return {std::vector<IdxT>(), std::vector<size_t>(0)};
}

}  // namespace cft

#endif /* CFT_INCLUDE_SPARSEBINMAT_HPP */