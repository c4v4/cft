#ifndef SCP_INCLUDE_MULTIPLIERS_HPP_
#define SCP_INCLUDE_MULTIPLIERS_HPP_
#include <vector>

#include "Instance.hpp"
#include "cft.hpp"

class LocalMultipliers : public std::vector<real_t> {
public:
    template <typename... Args>
    explicit LocalMultipliers(Args&&... _args) : std::vector<real_t>(std::forward<Args>(_args)...) { }

    [[nodiscard]] inline real_t compute_lb(SubInstance& subinst) const {
        auto& cols = subinst.get_cols();
        real_t LB = std::reduce(begin(), end(), 0.0);
        for (auto& col : cols) {
            real_t c_u = col.compute_lagr_cost(*this);
            if (c_u < 0.0) { LB += c_u; }
        }

        return LB;
    }
};


class GlobalMultipliers : public std::vector<real_t> {
public:
    GlobalMultipliers() : lb(REAL_LOWEST) { }
    explicit GlobalMultipliers(idx_t size) : std::vector<real_t>(size), lb(REAL_LOWEST) { }
    GlobalMultipliers(idx_t size, real_t val) : std::vector<real_t>(size, val), lb(REAL_LOWEST) { }
    GlobalMultipliers(const GlobalMultipliers& other) : std::vector<real_t>(other), lb(other.lb) { }
    GlobalMultipliers(GlobalMultipliers&& other) noexcept : std::vector<real_t>(std::move(other)), lb(other.lb) { }

    GlobalMultipliers& operator=(const GlobalMultipliers& other) = default;
    GlobalMultipliers& operator=(GlobalMultipliers&& other) = default;

    GlobalMultipliers(SubInstance& subinst, LocalMultipliers mult) : std::vector<real_t>(subinst.get_instance().get_nrows(), 0.0), lb(REAL_LOWEST) {
        for (idx_t i = 0; i < mult.size(); ++i) { (*this)[subinst.get_global_row_idx(i)] = mult[i]; }
        update_lb(subinst.get_instance());
    }

    inline void update_lb(Instance& inst) {

        real_t lb1 = inst.get_fixed_cost();
        real_t lb2 = 0.0;
        for (idx_t gi = 0; gi < size(); ++gi) {
            if (inst.is_row_active(gi)) { lb2 += (*this)[gi]; }
        }

        real_t lb3 = 0.0;
        auto& cols = inst.get_cols();
        for (idx_t gj : inst.get_active_cols()) {

            real_t c_u = cols[gj].get_cost();
            for (idx_t gi : cols[gj]) {
                if (inst.is_row_active(gi)) { c_u -= (*this)[gi]; }
            }

            if (c_u < 0.0) { lb3 += c_u; }
        }

        lb = lb1 + lb2 + lb3;
        // fmt::print("fixed {}, u {}, c_u {} = lb {}\n", lb1, lb2, lb3, lb);
    }

    [[nodiscard]] inline real_t get_lb() const { return lb; }


private:
    real_t lb;
};


#endif