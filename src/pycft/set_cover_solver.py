# SPDX-FileCopyrightText: 2025 Dominik Krupke <krupked@gmail.com>
# SPDX-License-Identifier: MIT

from pathlib import Path
from ._bindings import (
    Environment,
    parse_inst_and_initsol,
    run,
    Instance,
    Solution,
    CftResult,
)


class SetCoverSolver:
    """
    Defines a Set Cover Solver object that will solve the set cover problem using the CFT heuristic.
    The solver can be used incrementally by adding sets and then calling the solve method.
    If no new elements are added, the solver will use the previous solution as a starting point.

    All elements must be zero-indexed. As we are working with indices, all elements between 0 and the maximum
    element in the sets must appear in one of the sets for a feasible solution. If an element is not in any set,
    it will be considered as not covered and the solution will be infeasible.

    The sets are also indexed starting from 0. The index of a set is returned when adding a set and can be used
    to retrieve the solution.

    Example:
    ```python
    from pycft import SetCoverSolver

    solver = SetCoverSolver()
    # Set 0
    solver.add_set([0, 1, 2, 3, 4, 5, 6, 7, 8, 9], cost=10)
    # Set 1
    solver.add_set([0, 1, 2, 3, 4, 5], cost=5)
    # Set 2
    solver.add_set([0, 1, 2, 3, 4], cost=4)
    # Set 3
    solver.add_set([6, 7, 8, 9], cost=4)
    solver.solve()
    solution = solver.get_solution()
    print("The following sets have been selected:", solution)
    print("The cost of the solution is:", solver.get_cost())
    print("The lower bound of the solution is:", solver.get_lower_bound())
    ```
    """

    def __init__(self):
        self._result = None
        self._instance = Instance()
        self._initialized = False
        self._max_element = -1

    def add_set(self, elements: list[int], cost: float) -> int:
        """
        Add a set. Returns the index of the set which is also used in the solution.
        """
        if cost < 0:
            raise ValueError("Costs must be non-negative")
        if not all(0 <= e for e in elements):
            raise ValueError("Elements must be zero-indexed.")
        max_element = max(elements)
        if max_element > self._max_element:
            self._max_element = max_element
            self._result = (
                None  # in case there is a new element, the solution is no longer
            )
            # valid
        self._instance.add(elements, cost)
        self._initialized = False
        return len(self._instance.costs) - 1

    def from_file(self, filename: str | Path, parser: str = "RAIL") -> None:
        """
        Load an instance from a file.
        """
        env = Environment()
        env.time_limit = 10
        env.verbose = 2
        env.inst_path = str(filename)
        env.parser = parser
        fdata = parse_inst_and_initsol(env)
        self._instance = fdata.inst.copy()
        self._result.solution = fdata.init_sol.copy()
        print(self._result)
        self.num_elements = len(self._instance.costs)
        self._initialized = True  # a file is always considered initialized

    def solve(
        self,
        seed: int = 0,
        time_limit: float = float("inf"),
        verbose: int = 2,
        epsilon: float = 0.999,
        heur_iters: int = 250,
        alpha: float = 1.1,
        beta: float = 1.0,
        abs_subgrad_exit: float = 1.0,
        rel_subgrad_exit: float = 0.001,
        min_fixing=0.3,
    ) -> None:
        """
        Solves the set cover problem using the specified parameters.

        Parameters:
        seed (int): Seed for the random number generator. Default is 0.
        time_limit (int): Time limit in seconds. Default is 0 (no limit).
        verbose (int): Verbosity level. Default is 2.
        epsilon (float): Epsilon value for objective comparisons. Default is 0.999.
        heur_iters (int): Number of iterations for the heuristic phase. Default is 250.
        alpha (float): Relative fixing fraction increment. Default is 1.1.
        beta (float): Relative cutoff value to terminate Refinement. Default is 1.0.
        abs_subgrad_exit (float): Minimum LBs delta to trigger subgradient termination. Default is 1.0.
        rel_subgrad_exit (float): Minimum LBs gap to trigger subgradient termination. Default is 0.001.
        use_unit_costs (bool): Solve the given instance setting columns cost to one. Default is False.

        Returns:
        None
        """
        env = Environment()
        env.seed = seed
        env.time_limit = time_limit
        env.verbose = verbose
        env.epsilon = epsilon
        env.heur_iters = heur_iters
        env.alpha = alpha
        env.beta = beta
        env.abs_subgrad_exit = abs_subgrad_exit
        env.rel_subgrad_exit = rel_subgrad_exit
        env.min_fixing = min_fixing
        if not self._initialized:
            self._instance.rows.clear()
            self._instance.prepare()
            self._initialized = True
        init_sol = Solution() if self._result is None else self._result.sol
        self._result = run(env, self._instance, init_sol).copy()

    def get_solution(self) -> list[int] | None:
        """
        Return the indices of the selected sets in the solution.
        """
        if self._result is None:
            return None
        return list(self._result.sol.idxs)

    def get_cost(self) -> float | None:
        """
        Return the cost of the solution.
        """
        if self._result is None:
            return None
        return self._result.sol.cost

    def get_lower_bound(self) -> float:
        """
        Return a lower bound on the optimal solution.
        """
        if self._result is None:
            return 0
        return self._result.dual.cost

    def get_dual_multipliers(self) -> list[float] | None:
        """
        Return the dual multipliers of the solution.
        """
        if self._result is None:
            return None
        return list(self._result.dual.mults)
