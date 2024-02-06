#ifndef CAV_INCLUDE_INSTANCE_HPPCFT_
#define CAV_INCLUDE_INSTANCE_HPPCFT_

#include "SparseBinMat.hpp"
#include "cft.hpp"

namespace cft {

struct Instance {
    SparseBinMat<cidx_t> cols      = {};
    std::vector<real_t>  col_costs = {};

    std::vector<cidx_t> active_cols;
    std::vector<cidx_t> fixed_cols;

    ridx_t            nrows        = {};
    std::vector<bool> active_rows  = {};
    ridx_t            nactive_rows = {};
    real_t            fixed_cost   = {};
};
}  // namespace cft

#endif /* CAV_INCLUDE_INSTANCE_HPPCFT_ */
