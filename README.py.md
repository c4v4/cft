<!--
SPDX-FileCopyrightText: 2025 Dominik Krupke <krupked@gmail.com>
SPDX-License-Identifier: MIT
 -->

# Python Bindings for the AC-CFT Set Cover Heuristic



## Install

We will publish the package on PyPI soon. For now, you can install the package by cloning the repository and running the following command in the root directory:

```bash
pip install --verbose .
```

## Usage

To use the `SetCoverSolver`, first create an instance of the solver. You can then add sets with their respective costs and solve the set cover problem. The solver will find the optimal selection of sets that covers all elements at the minimum cost.

Here is an example:

```python
from pyaccft import SetCoverSolver

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

