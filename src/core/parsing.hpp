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

#ifndef CFT_SRC_INSTANCE_PARSING_HPP
#define CFT_SRC_INSTANCE_PARSING_HPP


#include <cassert>
#include <stdexcept>
#include <vector>

#include "core/Instance.hpp"
#include "core/cft.hpp"
#include "utils/SparseBinMat.hpp"
#include "utils/StringView.hpp"
#include "utils/limits.hpp"
#include "utils/parse_utils.hpp"

namespace cft {

struct FileData {
    Instance            inst;
    std::vector<cidx_t> warmstart;
};

inline Instance parse_scp_instance(std::string const& path) {
    auto file_iter = FileLineIterator(path);
    auto line_view = file_iter.next();
    auto inst      = Instance();

    // Read nrows & ncols
    ridx_t nrows = string_to<ridx_t>::consume(line_view);
    cidx_t ncols = string_to<cidx_t>::consume(line_view);
    if (!line_view.empty())
        throw std::invalid_argument("Invalid file format: not a SCP instance?");

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
        if (!line_view.empty())
            throw std::invalid_argument("Invalid file format: not a SCP instance?");

        for (ridx_t n = 0; n < i_ncols; ++n) {
            if (line_view.empty())
                line_view = file_iter.next();
            cidx_t cidx = string_to<cidx_t>::consume(line_view);
            assert(0 < cidx && cidx <= ncols);
            if (cidx <= 0 || ncols < cidx)
                throw std::invalid_argument("Invalid column index: not a SCP instance?");
            cols[cidx - 1].push_back(i);
        }
    }

    for (auto const& col : cols) {
        for (ridx_t i : col)
            inst.cols.idxs.push_back(i);
        inst.cols.begs.push_back(inst.cols.idxs.size());
    }

    inst.solcosts = std::vector<real_t>(ncols, limits<real_t>::max());
    inst.cols     = SparseBinMat<ridx_t>();
    for (auto& col : cols)
        inst.cols.push_back(col);

    fill_rows_from_cols(inst.cols, nrows, inst.rows);
    return inst;
}

inline Instance parse_rail_instance(std::string const& path) {
    auto file_iter = FileLineIterator(path);
    auto line_view = file_iter.next();
    auto inst      = Instance();

    // Read nrows & ncols
    ridx_t nrows = string_to<ridx_t>::consume(line_view);
    cidx_t ncols = string_to<cidx_t>::consume(line_view);
    if (!line_view.empty())
        throw std::invalid_argument("Invalid file format: not a RAIL instance?");
    for (cidx_t j = 0; j < ncols; j++) {
        line_view = file_iter.next();
        inst.costs.push_back(string_to<real_t>::consume(line_view));
        auto j_nrows = string_to<ridx_t>::consume(line_view);

        for (size_t n = 0; n < j_nrows; n++)
            inst.cols.idxs.push_back(string_to<cidx_t>::consume(line_view) - 1);
        inst.cols.begs.push_back(inst.cols.idxs.size());
    }
    inst.solcosts = std::vector<real_t>(ncols, limits<real_t>::max());

    fill_rows_from_cols(inst.cols, nrows, inst.rows);
    return inst;
}

inline FileData parse_cvrp_instance(std::string const& path) {
    auto file_iter = FileLineIterator(path);
    auto line_view = file_iter.next();
    auto fdata     = FileData();

    // Read nrows & ncols
    ridx_t nrows = string_to<ridx_t>::consume(line_view);
    cidx_t ncols = string_to<cidx_t>::consume(line_view);
    if (!line_view.empty())
        throw std::invalid_argument("Invalid file format: not a CVRP instance?");

    for (cidx_t j = 0; j < ncols; j++) {
        line_view = file_iter.next();
        fdata.inst.costs.push_back(string_to<real_t>::consume(line_view));
        fdata.inst.solcosts.push_back(string_to<real_t>::consume(line_view));
        while (!line_view.empty())
            fdata.inst.cols.idxs.push_back(string_to<ridx_t>::consume(line_view));
        fdata.inst.cols.begs.push_back(fdata.inst.cols.idxs.size());
    }

    auto warmstart = std::vector<cidx_t>();
    line_view      = file_iter.next();
    while (!line_view.empty())
        warmstart.push_back(string_to<cidx_t>::consume(line_view));

    fill_rows_from_cols(fdata.inst.cols, nrows, fdata.inst.rows);
    return fdata;
}

}  // namespace cft


#endif /* CFT_SRC_INSTANCE_PARSING_HPP */