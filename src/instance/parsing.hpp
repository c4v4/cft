// Copyright (c) 2024 Francesco Cavaliere
// This program is free software: you can redistribute it and/or modify it under the terms of the
// GNU General Public License as published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version. This program is distributed in the hope that it
// will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details. You should
// have received a copy of the GNU General Public License along with this program. If not, see
// <https://www.gnu.org/licenses/>.

#ifndef CFT_INCLUDE_PARSING_HPP
#define CFT_INCLUDE_PARSING_HPP

#include <cassert>
#include <vector>

#include "core/SparseBinMat.hpp"
#include "core/StringView.hpp"
#include "core/cft.hpp"
#include "core/limits.hpp"
#include "instance/parse_utils.hpp"

namespace cft {

struct InstanceData {
    ridx_t               nrows;
    std::vector<real_t>  costs;
    std::vector<real_t>  solcosts;
    SparseBinMat<cidx_t> cols;
    std::vector<cidx_t>  warmstart;
};

inline InstanceData make_instance_data() {
    return {0, {}, {}, make_sparse_bin_mat<ridx_t>(), {}};
}

inline InstanceData make_instance_data(cidx_t ncols, ridx_t nrows) {
    auto inst  = InstanceData{0, {}, {}, make_sparse_bin_mat<ridx_t>(), {}};
    inst.nrows = nrows;
    inst.costs.reserve(ncols);
    inst.solcosts.reserve(ncols);
    inst.cols.begs.reserve(ncols + 1);
    return inst;
}

inline InstanceData parse_scp_instance(std::string const& path) {
    auto file_iter = make_file_line_iterator(path);
    auto line_view = file_iter.next();

    // Read nrows & ncols
    ridx_t nrows = string_to<ridx_t>::consume(line_view);
    cidx_t ncols = string_to<cidx_t>::consume(line_view);
    auto   inst  = make_instance_data(ncols, nrows);
    assert(line_view.empty());

    for (cidx_t j = 0; j < ncols; ++j) {
        if (line_view.empty())
            line_view = file_iter.next();
        inst.costs.push_back(string_to<real_t>::consume(line_view));
    }

    // For each row: row_size
    //               list of row_size columns that cover the row
    auto cols = std::vector<std::vector<ridx_t>>(ncols);
    for (size_t i = 0; i < nrows; ++i) {
        line_view    = file_iter.next();
        auto i_ncols = string_to<cidx_t>::consume(line_view);
        assert(line_view.empty());

        for (ridx_t n = 0; n < i_ncols; ++n) {
            if (line_view.empty())
                line_view = file_iter.next();
            cidx_t cidx = string_to<cidx_t>::consume(line_view);
            assert(0 < cidx && cidx <= ncols);
            cols[cidx - 1].push_back(i);
        }
    }

    for (auto const& col : cols) {
        for (ridx_t i : col)
            inst.cols.idxs.push_back(i);
        inst.cols.begs.push_back(inst.cols.idxs.size());
    }

    inst.solcosts = std::vector<real_t>(ncols, limits<real_t>::max());
    inst.cols     = make_sparse_bin_mat<ridx_t>();
    for (auto& col : cols)
        inst.cols.push_back(col);

    return inst;
}

inline InstanceData parse_rail_instance(std::string const& path) {
    auto file_iter = make_file_line_iterator(path);
    auto line_view = file_iter.next();

    // Read nrows & ncols
    ridx_t nrows = string_to<ridx_t>::consume(line_view);
    cidx_t ncols = string_to<cidx_t>::consume(line_view);
    auto   inst  = make_instance_data(ncols, nrows);
    assert(line_view.empty());

    for (cidx_t j = 0; j < ncols; j++) {
        line_view = file_iter.next();
        inst.costs.push_back(string_to<real_t>::consume(line_view));
        auto j_nrows = string_to<ridx_t>::consume(line_view);

        for (size_t n = 0; n < j_nrows; n++)
            inst.cols.idxs.push_back(string_to<cidx_t>::consume(line_view) - 1);
        inst.cols.begs.push_back(inst.cols.idxs.size());
    }
    inst.solcosts = std::vector<real_t>(ncols, limits<real_t>::max());

    return inst;
}

inline InstanceData parse_cvrp_instance(std::string const& path) {
    auto file_iter = make_file_line_iterator(path);
    auto line_view = file_iter.next();

    // Read nrows & ncols
    ridx_t nrows = string_to<ridx_t>::consume(line_view);
    cidx_t ncols = string_to<cidx_t>::consume(line_view);
    auto   inst  = make_instance_data(ncols, nrows);
    assert(line_view.empty());

    for (cidx_t j = 0; j < ncols; j++) {
        line_view = file_iter.next();
        inst.costs.push_back(string_to<real_t>::consume(line_view));
        inst.solcosts.push_back(string_to<real_t>::consume(line_view));
        while (!line_view.empty())
            inst.cols.idxs.push_back(string_to<ridx_t>::consume(line_view));
        inst.cols.begs.push_back(inst.cols.idxs.size());
    }

    auto warmstart = std::vector<cidx_t>();
    line_view      = file_iter.next();
    while (!line_view.empty())
        warmstart.push_back(string_to<cidx_t>::consume(line_view));

    return inst;
}

}  // namespace cft


#endif /* CFT_INCLUDE_PARSING_HPP */
