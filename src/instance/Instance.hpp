#ifndef CFT_INCLUDE_INSTANCE_HPP
#define CFT_INCLUDE_INSTANCE_HPP


#include "core/SparseBinMat.hpp"
#include "core/cft.hpp"
#include "core/utility.hpp"
#include "instance/parsing.hpp"

#define CFT_REMOVED_IDX (cft::limits<cidx_t>::max())

namespace cft {

// A data structure representing an instance using sparse binary matrix representation.
struct Instance {
    // Current instance data
    SparseBinMat<ridx_t>             cols;
    std::vector<std::vector<cidx_t>> rows;
    std::vector<real_t>              costs;
    std::vector<real_t>              solcosts;

#ifndef NDEBUG
    void invariants_check() const {
        for (cidx_t j = 0; j < cols.size(); ++j) {
            assert("Col is empty" && !cols[j].empty());
            assert("Col does not exist" && j < cols.size());
            for (ridx_t i : cols[j])
                assert("Col not in row" && any(rows[i], [j](cidx_t rj) { return rj == j; }));
        }

        for (ridx_t i = 0; i < rows.size(); ++i) {
            assert("Row is empty" && !rows[i].empty());
            assert("Row does not exist" && i < rows.size());
            for (cidx_t j : rows[i])
                assert("Row not in col" && any(cols[j], [i](cidx_t ci) { return ci == i; }));
        }
    }
#endif
};

namespace {
    // Completes instance initialization by creating rows and orig_maps
    inline void complete_init(Instance& partial_inst, ridx_t nrows) {
        partial_inst.rows = std::vector<std::vector<cidx_t>>(nrows);

        for (cidx_t j = 0; j < partial_inst.cols.size(); ++j)
            for (ridx_t i : partial_inst.cols[j])
                partial_inst.rows[i].push_back(j);

        IF_DEBUG(partial_inst.invariants_check());
    }

    inline Instance make_instance(InstanceData&& inst_data) {
        Instance inst = {};
        inst.cols     = std::move(inst_data.cols);
        inst.costs    = std::move(inst_data.costs);
        inst.solcosts = std::move(inst_data.solcosts);

        complete_init(inst, inst_data.nrows);

        return inst;
    }

    inline Instance make_instance(InstanceData const& inst_data) {
        Instance inst = {};
        inst.cols     = inst_data.cols;
        inst.costs    = inst_data.costs;
        inst.solcosts = inst_data.solcosts;

        complete_init(inst, inst_data.nrows);

        return inst;
    }

    inline Instance make_tentative_core_instance(Instance const& inst, ridx_t min_row_coverage) {
        Instance core_inst = {};

        ridx_t nrows        = inst.rows.size();
        auto   row_coverage = std::vector<ridx_t>(nrows);
        ridx_t covered      = 0;

        // TODO(any): we may consider randomizing columns.
        // TODO(any): consider iterating over row indices and taking the first `min_row_coverage`
        // columns for every row.
        for (cidx_t j = 0; j < inst.cols.size(); ++j) {
            core_inst.cols.push_back(inst.cols[j]);
            core_inst.costs.push_back(inst.costs[j]);
            core_inst.solcosts.push_back(limits<real_t>::max());
            core_inst.costs.push_back(inst.costs[j]);

            // Update row coverage for early exit.
            for (ridx_t i : inst.cols[j]) {
                ++row_coverage[i];
                if (row_coverage[i] == min_row_coverage) {
                    ++covered;
                    if (covered == nrows)
                        goto done;
                }
            }
        }

    done:

        complete_init(core_inst, nrows);

        return core_inst;
    }
}  // namespace
}  // namespace cft

#endif /* CFT_INCLUDE_INSTANCE_HPP */
