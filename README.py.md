<!--
SPDX-FileCopyrightText: 2025 Dominik Krupke <krupked@gmail.com>
SPDX-License-Identifier: MIT
 -->

# Python Bindings for the AC-CFT Set Cover Heuristic

Implementation of the Caprara, Fischetti, and Toth algorithm for the [Set Covering problem](https://en.wikipedia.org/wiki/Set_cover_problem).
The original code is written in C++ and can be found [here](https://github.com/c4v4/cft).
This Python-packages wraps the C++ code using [pybind11](https://github.com/pybind/pybind11) and provides a simple interface to solve set cover instances.

*Caprara, A., Fischetti, M., & Toth, P. (1999). A Heuristic Method for the Set Covering Problem. Operations Research, 47(5), 730–743. [doi:10.1287/opre.47.5.730](https://doi.org/10.1287/opre.47.5.730)*

## Install

The package can be installed via pip:

```bash
pip install pycft
```

It should be precompiled for Windows, Linux, and MacOS. If not precompiled version is available, it should be able to compile itself if a C++-compiler is available on the system.

## Usage

To use the `SetCoverSolver`, first create an instance of the solver. You can then add sets with their respective costs and solve the set cover problem. The solver will find the optimal selection of sets that covers all elements at the minimum cost.

Here is an example:

```python
from pycft import SetCoverSolver

solver = SetCoverSolver()
# Add sets with their respective costs
solver.add_set([0, 1, 2, 3, 4, 5, 6, 7, 8, 9], cost=10)
solver.add_set([0, 1, 2, 3, 4, 5], cost=5)
solver.add_set([0, 1, 2, 3, 4], cost=4)
solver.add_set([6, 7, 8, 9], cost=4)

# Solve the set cover problem
solver.solve()

# Retrieve and print the solution
solution = solver.get_solution()
print("The following sets have been selected:", solution)
print("The cost of the solution is:", solver.get_cost())
print("The lower bound of the solution is:", solver.get_lower_bound())
```

Ensure that all elements are zero-indexed and that every element between 0 and the maximum element appears in at least one set for a feasible solution.

## Configuration

You can configure the solver by setting the following parameters in `solve`:

- seed (int): Seed for the random number generator. Default is 0.
- time_limit (int): Time limit in seconds. Default is 0 (no limit).
- verbose (int): Verbosity level. Default is 2.
- epsilon (float): Epsilon value for objective comparisons. Default is 0.999.
- heur_iters (int): Number of iterations for the heuristic phase. Default is 250.
- alpha (float): Relative fixing fraction increment. Default is 1.1.
- beta (float): Relative cutoff value to terminate Refinement. Default is 1.0.
- abs_subgrad_exit (float): Minimum LBs delta to trigger subgradient termination. Default is 1.0.
- rel_subgrad_exit (float): Minimum LBs gap to trigger subgradient termination. Default is 0.001.
- use_unit_costs (bool): Solve the given instance setting columns cost to one. Default is False.

