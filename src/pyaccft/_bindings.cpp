// pybind11
#include <pybind11/operators.h>  // To define operator overloading
#include <pybind11/pybind11.h>   // Basic pybind11 functionality
#include <pybind11/stl.h>        // Automatic conversion of vectors

// fmt
#include <fmt/core.h>

#include "../algorithms/Refinement.hpp"
#include "../core/CliArgs.hpp"
#include "../core/Instance.hpp"
#include "../core/cft.hpp"
#include "../core/parsing.hpp"
#include "../utils/print.hpp"

int _main(int argc, char const** argv) {

    try {
        auto env = cft::parse_cli_args(argc, argv);

        cft::print<1>(env, "CFT implementation by Luca Accorsi and Francesco Cavaliere.\n");
        cft::print<2>(env, "Compiled on " __DATE__ " at " __TIME__ ".\n\n");
        cft::print<3>(env, "Running with parameters set to:\n");
        cft::print_arg_values(env);

        auto fdata = cft::parse_inst_and_initsol(env);
        auto sol   = cft::run(env, fdata.inst, fdata.init_sol);
        cft::write_solution(env.sol_path, sol);
        cft::print<1>(env,
                      "CFT> Best solution {:.2f} time {:.2f}s\n",
                      sol.cost,
                      env.timer.elapsed<cft::sec>());

    } catch (std::exception const& e) {
        fmt::print(stderr, "\nCFT> ERROR: {}\n", e.what());
        std::fflush(stdout);
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

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
                 self.costs.push_back(cost);
                 return self.costs.size() - 1;
             })
        .def("copy", [](Instance const& a) { return Instance(a); })
        .def("fill_rows_from_cols", [](Instance& self, size_t no_elements) {
            self.rows.clear();
            fill_rows_from_cols(self.cols, no_elements, self.rows);
        });

    py::class_<Solution>(m, "Solution")
        .def(py::init<>())
        .def_readwrite("idxs", &Solution::idxs)
        .def_readwrite("cost", &Solution::cost)
        .def_readwrite("lower_bound", &Solution::lower_bound)
        .def("copy", [](Solution const& a) { return Solution(a); })
        .def("__repr__", [](Solution const& a) {
            return fmt::format("Solution(idxs={}, cost={}, lower_bound={})",
                               a.idxs,
                               a.cost,
                               a.lower_bound);
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