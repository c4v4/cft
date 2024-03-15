#include <catch2/catch.hpp>
#include <cstring>

#include "greedy/Greedy.hpp"

namespace cft {

// TODO(cava): make tests for these functions

// real_t Greedy::operator()(Instance const&            inst,
//                           std::vector<real_t> const& lagr_mult,
//                           std::vector<cidx_t>&       sol,
//                           real_t                     upper_bound  = limits<real_t>::max(),
//                           cidx_t                     max_sol_size = limits<cidx_t>::max());

// inline real_t init_redund_set(RedundancyData&            red_data,
//                               Instance const&            inst,
//                               std::vector<cidx_t> const& sol,
//                               real_t                     ub,
//                               Sorter&                    sorter);

// inline real_t heuristic_removal(RedundancyData& red_set, Instance const& inst, real_t lb);

// inline real_t enumeration_removal(RedundancyData& red_set, Instance const& inst, real_t lb);

// void Scores::init_scores(Instance const&            inst,
//                          std::vector<real_t> const& lagr_mult,
//                          CoverBits const&           cover_bits,
//                          cidx_t                     sol_size,
//                          Sorter&                    sorter);

// cidx_t Scores::extract_minscore_col(Instance const&            inst,
//                                     std::vector<real_t> const& lagr_mult,
//                                     CoverBits&                 cover_bits,
//                                     Sorter&                    sorter);

// static Enumerator<>::invoke(Instance const& inst,
//                             RedundancyData& red_data,
//                             real_t          lb,
//                             bool*           vars,
//                             bool*           sol);

}  // namespace cft