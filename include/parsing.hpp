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

#ifndef CFT_INCLUDE_PARSING_HPP
#define CFT_INCLUDE_PARSING_HPP

#include <cassert>
#include <vector>

#include "SparseBinMat.hpp"
#include "StringView.hpp"
#include "cft.hpp"
#include "limits.hpp"
#include "parse_utils.hpp"

namespace cft {

struct InstanceData {
    ridx_t               nrows;
    std::vector<real_t>  solcosts;
    SparseBinMat<ridx_t> cols;
    std::vector<cidx_t>  warmstart;
};

inline InstanceData parse_scp_instance(std::string const& path) {
    auto inst      = InstanceData{};
    auto file_iter = make_file_line_iterator(path);

    // Read nrows & ncols
    auto line_view = file_iter.next();
    inst.nrows     = string_to<ridx_t>::consume(line_view);
    cidx_t ncols   = string_to<cidx_t>::consume(line_view);
    assert(line_view.empty());

    auto cobjs = std::vector<real_t>(ncols);
    for (cidx_t j = 0; j < ncols; ++j) {
        if (line_view.empty())
            line_view = file_iter.next();
        cobjs[j] = string_to<real_t>::consume(line_view);
    }

    // For each row: row_size
    //               list of row_size columns that cover the row
    auto cols = std::vector<std::vector<ridx_t>>(ncols);
    for (size_t i = 0; i < inst.nrows; ++i) {
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

    inst.solcosts = std::vector<real_t>(ncols, limits<real_t>::max());
    inst.cols     = make_sparse_bin_mat<ridx_t>();
    for (cidx_t j = 0; j < ncols; ++j)
        inst.cols.add_col(cols[j], cobjs[j]);

    return inst;
}

inline InstanceData parse_rail_instance(std::string const& path) {
    auto inst      = InstanceData{};
    auto file_iter = make_file_line_iterator(path);

    // Read nrows & ncols
    auto line_view = file_iter.next();
    inst.nrows     = string_to<ridx_t>::consume(line_view);
    cidx_t ncols   = string_to<cidx_t>::consume(line_view);
    assert(line_view.empty());

    auto cobjs = std::vector<real_t>(ncols);
    auto cbegs = std::vector<size_t>();
    auto cidxs = std::vector<ridx_t>();
    for (auto j = 0UL; j < ncols; j++) {
        line_view    = file_iter.next();
        cobjs[j]     = string_to<real_t>::consume(line_view);
        auto j_nrows = string_to<ridx_t>::consume(line_view);

        cbegs.push_back(cidxs.size());
        for (size_t n = 0; n < j_nrows; n++)
            cidxs.push_back(string_to<cidx_t>::consume(line_view) - 1);
    }

    cbegs.push_back(cidxs.size());
    inst.cols     = make_sparse_bin_mat(std::move(cidxs), std::move(cbegs), std::move(cobjs));
    inst.solcosts = std::vector<real_t>(ncols, limits<real_t>::max());

    return inst;
}

inline InstanceData parse_cvrp_instance(std::string const& path) {
    auto inst      = InstanceData{};
    auto file_iter = make_file_line_iterator(path);

    // Read nrows & ncols
    auto line_view = file_iter.next();
    inst.nrows     = string_to<ridx_t>::consume(line_view);
    cidx_t ncols   = string_to<cidx_t>::consume(line_view);
    assert(line_view.empty());

    auto cbegs    = std::vector<size_t>();
    auto cidxs    = std::vector<ridx_t>();
    auto cobjs    = std::vector<real_t>(ncols);
    inst.solcosts = std::vector<real_t>(ncols);

    for (cidx_t j = 0; j < ncols; j++) {
        line_view        = file_iter.next();
        cobjs[j]         = string_to<real_t>::consume(line_view);
        inst.solcosts[j] = string_to<real_t>::consume(line_view);

        cbegs.push_back(cidxs.size());
        while (!line_view.empty())
            cidxs.push_back(string_to<ridx_t>::consume(line_view));
    }

    cbegs.push_back(cidxs.size());
    inst.cols = make_sparse_bin_mat(std::move(cidxs), std::move(cbegs), std::move(cobjs));

    auto warmstart = std::vector<cidx_t>();
    line_view      = file_iter.next();
    while (!line_view.empty())
        warmstart.push_back(string_to<cidx_t>::consume(line_view));

    return inst;
}

}  // namespace cft


#endif /* CFT_INCLUDE_PARSING_HPP */
