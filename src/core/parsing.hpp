// SPDX-FileCopyrightText: 2024 Francesco Cavaliere <francescocava95@gmail.com>
// SPDX-License-Identifier: MIT

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
#include "utils/print.hpp"

#ifndef NDEBUG
#include "core/utils.hpp"
#endif

namespace cft {

namespace local { namespace {
    struct InstSize {
        ridx_t rows;
        cidx_t cols;
    };

    InstSize read_nrows_and_ncols(FileLineIterator& file_iter) {
        auto line_view = file_iter.next();
        auto num       = InstSize();
        num.rows       = string_to<ridx_t>::consume(line_view);
        num.cols       = string_to<cidx_t>::consume(line_view);
        if (!line_view.empty())
            throw std::invalid_argument("Invalid file format: too many values in the first line.");
        return num;
    }

#ifndef NDEBUG
    inline void mps_epilogue_check(FileLineIterator&                              file_iter,
                                   std::unordered_map<std::string, ridx_t> const& rows_map) {
        // Check RHSs
        auto line_view = file_iter.next();
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
    }
#endif
}  // namespace
}  // namespace local

inline Instance parse_scp_instance(std::string const& path) {
    auto file_iter = FileLineIterator(path);
    auto num       = local::read_nrows_and_ncols(file_iter);

    auto line_view = StringView();
    auto inst      = Instance();
    for (cidx_t j = 0_C; j < num.cols; ++j) {
        if (line_view.empty())
            line_view = file_iter.next();
        inst.costs.push_back(string_to<real_t>::consume(line_view));
    }

    // For each row: row_size
    //               list of row_size columns that cover the row
    auto cols = std::vector<std::vector<ridx_t>>(num.cols);
    for (ridx_t i = 0_R; i < num.rows; ++i) {
        line_view    = file_iter.next();
        auto i_ncols = string_to<cidx_t>::consume(line_view);
        if (!line_view.empty())
            throw std::invalid_argument("Invalid file format: not a SCP instance?");

        for (cidx_t n = 0_C; n < i_ncols; ++n) {
            if (line_view.empty())
                line_view = file_iter.next();
            cidx_t cidx = string_to<cidx_t>::consume(line_view);
            assert(0_C < cidx && cidx <= num.cols);
            if (cidx <= 0_C || num.cols < cidx)
                throw std::invalid_argument("Invalid column index: not a SCP instance?");
            cols[cidx - 1_C].push_back(i);
        }
    }

    for (auto const& col : cols) {
        for (ridx_t i : col)
            inst.cols.idxs.push_back(i);
        inst.cols.begs.push_back(csize(inst.cols.idxs));
    }

    inst.cols = SparseBinMat<ridx_t>();
    for (auto& col : cols)
        inst.cols.push_back(col);

    fill_rows_from_cols(inst.cols, num.rows, inst.rows);
    return inst;
}

inline Instance parse_rail_instance(std::string const& path) {
    auto file_iter = FileLineIterator(path);
    auto num       = local::read_nrows_and_ncols(file_iter);

    auto line_view = StringView();
    auto inst      = Instance();
    for (cidx_t j = 0_C; j < num.cols; j++) {
        line_view = file_iter.next();
        inst.costs.push_back(string_to<real_t>::consume(line_view));
        auto j_nrows = string_to<ridx_t>::consume(line_view);
        auto tokens  = split(line_view);
        if (rsize(tokens) != j_nrows)
            throw std::invalid_argument("Invalid file format: not a RAIL instance?");

        for (ridx_t n = 0_R; n < j_nrows; n++) {
            inst.cols.idxs.push_back(string_to<ridx_t>::parse(tokens[n]) - 1_R);
            if (inst.cols.idxs.back() >= num.rows)
                throw std::invalid_argument("Invalid file format: not a RAIL instance?");
        }
        inst.cols.begs.push_back(csize(inst.cols.idxs));
    }

    fill_rows_from_cols(inst.cols, num.rows, inst.rows);
    return inst;
}

struct FileData {
    Instance inst;
    Solution init_sol;
};

inline FileData parse_cvrp_instance(std::string const& path) {
    auto file_iter = FileLineIterator(path);
    auto num       = local::read_nrows_and_ncols(file_iter);

    auto line_view = StringView();
    auto fdata     = FileData();
    for (cidx_t j = 0_C; j < num.cols; j++) {
        line_view = file_iter.next();
        fdata.inst.costs.push_back(string_to<real_t>::consume(line_view));

        real_t solcost = string_to<real_t>::consume(line_view);
        if (solcost < fdata.inst.costs.back())
            throw std::invalid_argument("Invalid file format: not a CVRP instance?");

        while (!line_view.empty()) {
            fdata.inst.cols.idxs.push_back(string_to<ridx_t>::consume(line_view));
            if (fdata.inst.cols.idxs.back() >= num.rows)
                throw std::invalid_argument("Invalid file format: not a CVRP instance?");
        }
        fdata.inst.cols.begs.push_back(csize(fdata.inst.cols.idxs));
    }

    line_view = file_iter.next();
    while (!line_view.empty()) {
        cidx_t j = string_to<cidx_t>::consume(line_view);
        fdata.init_sol.idxs.push_back(j);
        fdata.init_sol.cost += fdata.inst.costs[j];
    }

    fill_rows_from_cols(fdata.inst.cols, num.rows, fdata.inst.rows);
    return fdata;
}

// Note: not a complete mps parser, best effort to parse a SCP instance, but can probably fail with
// some formats, or parse non SCP instances as SCP.
inline Instance parse_mps_instance(std::string const& path) {
    auto file_iter = FileLineIterator(path);
    auto inst      = Instance();

    auto   line_view    = file_iter.next();
    size_t down_counter = 10;
    while (line_view != "ROWS" && down_counter-- > 0)
        line_view = file_iter.next();
    if (line_view != "ROWS")
        throw std::invalid_argument("Invalid file format: not a MPS instance?");

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

    CFT_IF_DEBUG(local::mps_epilogue_check(file_iter, rows_map));
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

inline FileData parse_inst_and_initsol(Environment const& env) {
    auto fdata = FileData();

    if (env.parser == CFT_RAIL_PARSER) {
        print<1>(env, "CFT> Parsing RAIL instance from {}\n\n", env.inst_path);
        fdata.inst = parse_rail_instance(env.inst_path);

    } else if (env.parser == CFT_SCP_PARSER) {
        print<1>(env, "CFT> Parsing SCP instance from {}\n\n", env.inst_path);
        fdata.inst = parse_scp_instance(env.inst_path);

    } else if (env.parser == CFT_CVRP_PARSER) {
        print<1>(env, "CFT> Parsing CVRP instance from {}\n\n", env.inst_path);
        fdata = parse_cvrp_instance(env.inst_path);

    } else if (env.parser == CFT_MPS_PARSER) {
        print<1>(env, "CFT> Parsing MPS instance from {}\n\n", env.inst_path);
        fdata.inst = parse_mps_instance(env.inst_path);

    } else {
        print<1>(env, "CFT> Parser {} does not exists.\n\n", env.parser);
        throw std::runtime_error("Parser does not exists.");
    }

    if (!env.initsol_path.empty()) {
        fdata.init_sol = parse_solution(env.initsol_path);
        CFT_IF_DEBUG(check_inst_solution(fdata.inst, fdata.init_sol));
    }

    if (env.use_unit_costs) {
        fdata.inst.costs.assign(csize(fdata.inst.costs), 1.0_F);
        fdata.init_sol.cost = as_real(size(fdata.init_sol.idxs));
    }

    print<1>(env, "CFT> Instance size: {} x {}.\n", rsize(fdata.inst.rows), csize(fdata.inst.cols));
    return fdata;
}


}  // namespace cft


#endif /* CFT_SRC_INSTANCE_PARSING_HPP */
