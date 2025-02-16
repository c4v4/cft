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