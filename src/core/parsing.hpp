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

#include <fmt/ostream.h>

#include <stdexcept>
#include <unordered_map>
#include <vector>

#include "core/Instance.hpp"
#include "core/cft.hpp"
#include "utils/SparseBinMat.hpp"
#include "utils/StringView.hpp"
#include "utils/assert.hpp"  // IWYU pragma:  keep
#include "utils/limits.hpp"
#include "utils/parse_utils.hpp"

namespace cft {

struct FileData {
    Instance inst;
    Solution init_sol;
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

    for (cidx_t j = 0_C; j < ncols; ++j) {
        if (line_view.empty())
            line_view = file_iter.next();
        inst.costs.push_back(string_to<real_t>::consume(line_view));
    }

    // For each row: row_size
    //               list of row_size columns that cover the row
    auto cols = std::vector<std::vector<ridx_t>>(ncols);
    for (ridx_t i = 0_R; i < nrows; ++i) {
        line_view    = file_iter.next();
        auto i_ncols = string_to<cidx_t>::consume(line_view);
        if (!line_view.empty())
            throw std::invalid_argument("Invalid file format: not a SCP instance?");

        for (cidx_t n = 0_C; n < i_ncols; ++n) {
            if (line_view.empty())
                line_view = file_iter.next();
            cidx_t cidx = string_to<cidx_t>::consume(line_view);
            assert(0_C < cidx && cidx <= ncols);
            if (cidx <= 0_C || ncols < cidx)
                throw std::invalid_argument("Invalid column index: not a SCP instance?");
            cols[cidx - 1].push_back(i);
        }
    }

    for (auto const& col : cols) {
        for (ridx_t i : col)
            inst.cols.idxs.push_back(i);
        inst.cols.begs.push_back(csize(inst.cols.idxs));
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
    for (cidx_t j = 0_C; j < ncols; j++) {
        line_view = file_iter.next();
        inst.costs.push_back(string_to<real_t>::consume(line_view));
        auto j_nrows = string_to<ridx_t>::consume(line_view);

        for (ridx_t n = 0_R; n < j_nrows; n++)
            inst.cols.idxs.push_back(string_to<ridx_t>::consume(line_view) - 1_R);
        inst.cols.begs.push_back(csize(inst.cols.idxs));
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

    for (cidx_t j = 0_C; j < ncols; j++) {
        line_view = file_iter.next();
        fdata.inst.costs.push_back(string_to<real_t>::consume(line_view));
        fdata.inst.solcosts.push_back(string_to<real_t>::consume(line_view));
        while (!line_view.empty())
            fdata.inst.cols.idxs.push_back(string_to<ridx_t>::consume(line_view));
        fdata.inst.cols.begs.push_back(csize(fdata.inst.cols.idxs));
    }

    line_view = file_iter.next();
    while (!line_view.empty()) {
        cidx_t j = string_to<cidx_t>::consume(line_view);
        fdata.init_sol.idxs.push_back(j);
        fdata.init_sol.cost += fdata.inst.costs[j];
    }

    fill_rows_from_cols(fdata.inst.cols, nrows, fdata.inst.rows);
    return fdata;
}

inline Instance parse_mps_instance(std::string const& path) {
    auto file_iter = FileLineIterator(path);
    auto inst      = Instance();

    auto line_view = file_iter.next();
    while (line_view != "ROWS")
        line_view = file_iter.next();

    ridx_t nrows    = 0_R;
    auto   rows_map = std::unordered_map<std::string, ridx_t>();
    auto   obj_name = std::string();
    while (line_view != "COLUMNS") {
        auto tokens = split(line_view);
        if (!tokens.empty() && tokens[0] == "N")
            obj_name = tokens[1].to_cpp_string();
        if (!tokens.empty() && (tokens[0] == "G" || tokens[0] == "E" || tokens[0] == "L"))
            rows_map[tokens[1].to_cpp_string()] = nrows++;
        line_view = file_iter.next();
    }

    auto prev_col_name = std::string();
    line_view          = file_iter.next();
    inst.cols.begs.clear();
    while (line_view != "RHS") {
        auto tokens = split(line_view);
        // Best effort to detect columns header line
        if (tokens.size() < 3 || (std::isdigit(tokens[2][0]) == 0 && tokens[2][0] != '-')) {
            line_view = file_iter.next();
            continue;
        }

        if (tokens[0] != prev_col_name) {  // new column
            prev_col_name = tokens[0].to_cpp_string();
            inst.cols.begs.push_back(csize(inst.cols.idxs));
            inst.solcosts.push_back(limits<real_t>::max());
            inst.costs.push_back(limits<real_t>::max());
        }

        for (size_t t = 1; t < tokens.size(); t += 2) {
            if (tokens[t] == obj_name)
                inst.costs.back() = string_to<real_t>::parse(tokens[t + 1]);
            else {
                auto row_name = tokens[t];
                assert(rows_map.find(row_name.to_cpp_string()) != rows_map.end());
                assert(tokens[t + 1] == "1" || tokens[t + 1] == "-1");
                inst.cols.idxs.push_back(rows_map[row_name.to_cpp_string()]);
            }
        }

        line_view = file_iter.next();
    }
    inst.cols.begs.push_back(csize(inst.cols.idxs));


#ifndef NDEBUG
    // Check RHSs
    line_view = file_iter.next();
    while (line_view != "BOUNDS") {
        line_view   = trim(line_view);
        auto tokens = split(line_view);
        assert(tokens[0] == "rhs" || tokens[0] == "RHS");

        for (size_t t = 1; t < tokens.size(); t += 2) {
            auto row_name = tokens[t];
            assert(rows_map.find(row_name.to_cpp_string()) != rows_map.end());
            assert(tokens[t + 1] == "1" || tokens[t + 1] == "-1");
        }

        line_view = file_iter.next();
    }

    // TODO(any): check bounds
#endif

    fill_rows_from_cols(inst.cols, nrows, inst.rows);
    return inst;
}

inline Solution parse_solution(std::string const& path) {
    auto file_iter = FileLineIterator(path);
    auto line_view = file_iter.next();
    auto sol       = Solution();

    sol.cost = string_to<real_t>::consume(line_view);
    while (!line_view.empty())
        sol.idxs.push_back(string_to<cidx_t>::consume(line_view));

    return sol;
}

inline void write_solution(std::string const& path, Solution const& sol) {
    auto file = std::ofstream(path);
    if (!file.is_open())
        throw std::runtime_error("Failed to open file for writing: " + path);
    fmt::print(file, "{}", sol.cost);
    for (cidx_t j : sol.idxs)
        fmt::print(file, " {}", j);
    file.close();
}


}  // namespace cft


#endif /* CFT_SRC_INSTANCE_PARSING_HPP */
