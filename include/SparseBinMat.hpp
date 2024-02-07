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
    std::vector<real_t> objs;

    CFT_NODISCARD Span<IdxT*> get_col(cidx_t j) {
        assert(j < begs.size() - 1);
        assert(begs[j] < idxs.size());
        assert(begs[j + 1] < idxs.size());
        return {idxs.data() + begs[j], idxs.data() + begs[j + 1]};
    }

    CFT_NODISCARD Span<IdxT const*> get_col(cidx_t j) const {
        assert(j < begs.size());
        assert(begs[j] < idxs.size());
        assert(begs[j + 1] <= idxs.size());
        return {idxs.data() + begs[j], idxs.data() + begs[j + 1]};
    }

    CFT_NODISCARD real_t get_cost(cidx_t j) {
        assert(j < objs.size());
        return objs[j];
    }

    CFT_NODISCARD std::size_t size() const {
        return begs.size() - 1;
    }

    CFT_NODISCARD bool empty() const {
        return begs.size() == 1;
    }

    template <typename T>
    void for_each_nz(T&& op) {
        for (IdxT i : idxs)
            op(i);
    }

    void clear() {
        idxs.clear();
        begs.clear();
        begs.push_back(0);
    }

    template <typename T>
    void add_col(T const& elem, real_t cost) {
        idxs.insert(idxs.end(), elem.begin(), elem.end());
        begs.push_back(idxs.size());
        objs.push_back(cost);
    }
};

template <typename IdxT>
CFT_NODISCARD inline SparseBinMat<IdxT> make_sparse_bin_mat() {
    return {std::vector<IdxT>(), std::vector<size_t>(1, 0), std::vector<real_t>()};
}

template <typename IdxT>
CFT_NODISCARD inline SparseBinMat<IdxT> make_sparse_bin_mat(std::vector<IdxT>&&   idxs,
                                                            std::vector<size_t>&& begs,
                                                            std::vector<real_t>&& objs) {
    assert(begs.size() == objs.size() + 1);
    return {std::move(idxs), std::move(begs), std::move(objs)};
}

template <typename IdxT, typename T1, typename T2, typename T3>
CFT_NODISCARD inline SparseBinMat<IdxT> make_sparse_bin_mat(T1 const& idxs,
                                                            T2 const& begs,
                                                            T3 const& objs) {
    assert(begs.size() == objs.size() + 1);
    return {std::vector<IdxT>(idxs.begin(), idxs.end()),
            std::vector<size_t>(begs.begin(), begs.end()),
            std::vector<real_t>(objs.begin(), objs.end())};
}

}  // namespace cft

#endif /* CFT_INCLUDE_SPARSEBINMAT_HPP */
