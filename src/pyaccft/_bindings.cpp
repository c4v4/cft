// SPDX-FileCopyrightText: 2025 Dominik Krupke <krupked@gmail.com>
// SPDX-License-Identifier: MIT

// pybind11
#include <pybind11/operators.h>  // To define operator overloading
#include <pybind11/pybind11.h>   // Basic pybind11 functionality
#include <pybind11/stl.h>        // Automatic conversion of vectors

#include <stdexcept>

#include "../algorithms/Refinement.hpp"
#include "../core/Instance.hpp"
#include "../core/cft.hpp"
#include "../core/parsing.hpp"

namespace local { namespace {
    void check_and_fill_instance(cft::Instance& instance) {
        auto& cols = instance.cols;
        auto& rows = instance.rows;
        using namespace cft;
        size_t n = 0;

        // Determine the maximum index in cols
        for (cidx_t j = 0_C; j < csize(cols); ++j)
            for (ridx_t i : cols[j])
                n = std::max(n, static_cast<size_t>(i + 1));

        // Guard against the user entering huge indices because they
        // might have misunderstood the 0-based indexing.
        if (n > cols.idxs.size()) {
            throw std::runtime_error(
                fmt::format("Item index out of bounds. Maximum index is {} but "
                            "size is {}. Please make sure that the n items are "
                            "indexed from 0 to n-1.",
                            n,
                            cols.idxs.size()));
        }

        // Check if every element (row) is in at least one column
        std::vector<bool> in_col(n, false);
        // Mark every element that is in a column (set)
        for (cidx_t j = 0_C; j < csize(cols); ++j)
            for (ridx_t i : cols[j])
                in_col[i] = true;
        // Check if every element is in at least one column
        for (size_t i = 0; i < n; ++i)
            if (!in_col[i])
                throw std::runtime_error(fmt::format("Item {} not contained in any set.", i));

        // Fill rows from columns
        fill_rows_from_cols(cols, n, rows);
    }
}  // namespace
}  // namespace local

// Pybind11 module definitions
PYBIND11_MODULE(_bindings, m) {
    namespace py = pybind11;
    using namespace cft;

    m.doc() = "Example of PyBind11 and CGAL.";  // Optional module docstring

    py::class_<Environment>(m, "Environment", "The Environment for the accft solver.")
        .def(py::init<>())
        .def_readwrite("inst_path", &Environment::inst_path)
        .def_readwrite("sol_path", &Environment::sol_path)
        .def_readwrite("initsol_path", &Environment::initsol_path)
        .def_readwrite("parser", &Environment::parser)
        .def_readwrite("seed", &Environment::seed)
        .def_readwrite("time_limit", &Environment::time_limit)
        .def_readwrite("verbose", &Environment::verbose)
        .def_readwrite("epsilon", &Environment::epsilon)
        .def_readwrite("heur_iters", &Environment::heur_iters)
        .def_readwrite("alpha", &Environment::alpha)
        .def_readwrite("beta", &Environment::beta)
        .def_readwrite("abs_subgrad_exit", &Environment::abs_subgrad_exit)
        .def_readwrite("rel_subgrad_exit", &Environment::rel_subgrad_exit)
        .def_readwrite("use_unit_costs", &Environment::use_unit_costs)
        .def_readwrite("min_fixing", &Environment::min_fixing)
        .def("__repr__", [](Environment const& a) {
            return fmt::format("Environment(inst_path='{}', sol_path='{}', initsol_path='{}', "
                               "parser={}, "
                               "seed={}, time_limit={}, verbose={}, epsilon={}, heur_iters={}, "
                               "alpha={}, beta={}, abs_subgrad_exit={}, rel_subgrad_exit={}, "
                               "use_unit_costs={})",
                               a.inst_path,
                               a.sol_path,
                               a.initsol_path,
                               a.parser,
                               a.seed,
                               a.time_limit,
                               a.verbose,
                               a.epsilon,
                               a.heur_iters,
                               a.alpha,
                               a.beta,
                               a.abs_subgrad_exit,
                               a.rel_subgrad_exit,
                               a.use_unit_costs);
        });
    py::class_<SparseBinMat<ridx_t>>(m, "SparseBinMat")
        .def(py::init<>())
        .def_readwrite("idxs", &SparseBinMat<ridx_t>::idxs)
        .def_readwrite("begs", &SparseBinMat<ridx_t>::begs)
        .def("__getitem__", [](SparseBinMat<ridx_t>& self, std::size_t i) { return self[i]; })
        .def("__len__", &SparseBinMat<ridx_t>::size)
        .def("__repr__",
             [](SparseBinMat<ridx_t> const& a) {
                 return fmt::format("SparseBinMat(idxs={}, begs={})", a.idxs, a.begs);
             })
        .def("clear", &SparseBinMat<ridx_t>::clear)
        .def("push_back",
             static_cast<void (SparseBinMat<ridx_t>::*)(std::vector<ridx_t> const&)>(
                 &SparseBinMat<ridx_t>::push_back));


    py::class_<Instance>(m, "Instance")
        .def(py::init<>())
        .def_readwrite("cols", &Instance::cols)
        .def_readwrite("rows", &Instance::rows)
        .def_readwrite("costs", &Instance::costs)
        .def("add",
             [](Instance& self, std::vector<ridx_t> const& col, real_t cost) {
                 self.cols.push_back(col);
                 if (cost < 0.0)
                     throw std::runtime_error("Costs must be non-negative.");
                 self.costs.push_back(cost);
                 return self.costs.size() - 1;
             })
        .def("copy", [](Instance const& a) { return a; })
        .def("prepare", [](Instance& self) { ::local::check_and_fill_instance(self); });


    py::class_<CftResult>(m, "CftResult")
        .def(py::init<>())
        .def_readwrite("sol", &CftResult::sol)
        .def_readwrite("dual", &CftResult::dual)
        .def("copy", [](CftResult const& a) { return a; })
        .def("__repr__", [](CftResult const& a) {
            return fmt::format("CftResult(sol=({},{}), dual=({},{}))",
                               a.sol.cost,
                               a.sol.cost,
                               a.dual.mults,
                               a.dual.lb);
        });

    py::class_<Solution>(m, "Solution")
        .def(py::init<>())
        .def_readwrite("idxs", &Solution::idxs)
        .def_readwrite("cost", &Solution::cost)
        .def("copy", [](Solution const& a) { return a; })
        .def("__repr__", [](Solution const& a) {
            return fmt::format("Solution(idxs={}, cost={})", a.idxs, a.cost);
        });

    py::class_<DualState>(m, "DualState")
        .def(py::init<>())
        .def_readwrite("idxs", &DualState::mults)
        .def_readwrite("cost", &DualState::lb)
        .def("copy", [](DualState const& a) { return a; })
        .def("__repr__", [](DualState const& a) {
            return fmt::format("DualState(mults={}, lb={})", a.mults, a.lb);
        });


    py::class_<FileData>(m, "FileData")
        .def_readwrite("inst", &FileData::inst)
        .def_readwrite("init_sol", &FileData::init_sol);

    m.def("parse_inst_and_initsol",
          &parse_inst_and_initsol,
          "Parse the instance and initial solution.");

    m.def("fill_rows_from_cols", &fill_rows_from_cols, "Fill rows from columns.");


    m.def("run", &run, "Run the accft solver.");
}