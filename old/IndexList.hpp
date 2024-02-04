#ifndef SCP_INCLUDE_INDEXLIST_HPP_
#define SCP_INCLUDE_INDEXLIST_HPP_

#include <algorithm>
#include <vector>

#include "cft.hpp"

class IndexList : public std::vector<idx_t> {
public:
    template <typename... Args>
    explicit IndexList(Args&&... _args) : std::vector<idx_t>(std::forward<Args>(_args)...) { }
};


class Column : public IndexList {

public:
    Column() : c(0.0), sol_c(0.0), c_u(0.0) { }
    template <typename... Args>
    explicit Column(real_t _cost, real_t _solcost, Args&&... _args) : IndexList(std::forward<Args>(_args)...), c(_cost), sol_c(_solcost), c_u(_cost) { }

    [[nodiscard]] inline auto get_cost() const { return c; }
    inline void set_cost(real_t new_c) {
        c = new_c;
        c_u = new_c;
    }

    [[nodiscard]] inline auto get_solcost() const { return sol_c; }
    inline void set_solcost(real_t new_c) { sol_c = new_c; }

    inline void set_cu(real_t new_c) { c_u = new_c; }

    [[nodiscard]] inline auto get_cu() const { return c_u; }

    [[nodiscard]] inline auto compute_lagr_cost(const std::vector<real_t>& u) const {
        real_t local_c_u = c;
        for (auto i : *this) {
            assert(i < u.size());
            local_c_u -= u[i];
        }
        return local_c_u;
    }

    inline auto compute_lagr_cost(const std::vector<real_t>& u) {
        c_u = c;
        for (auto i : *this) {
            assert(i < u.size());
            c_u -= u[i];
        }
        return c_u;
    }

    inline auto update_cu(const real_t delta_u) { return c_u -= delta_u; }

    bool operator==(const Column& other) const {
        if (c != other.c || sol_c != other.sol_c || size() != other.size()) { return false; }
        for (idx_t n = 0; n < size(); ++n) {
            if ((*this)[n] != other[n]) { return false; }
        }
        return true;
    }

private:
    real_t c;
    real_t sol_c;
    real_t c_u;
};


class Row : public IndexList {
public:
    template <typename... Args>
    explicit Row(Args&&... _args) : IndexList(std::forward<Args>(_args)...) { }

    /*void remove_elem(idx_t) = delete;
    inline auto remove_col(idx_t _col) { IndexList::remove_elem(_col); }*/

private:
};


#endif