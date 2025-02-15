from pathlib import Path
from ._bindings import (
    Environment,
    parse_inst_and_initsol,
    run,
    Instance,
    Solution,
)

__all__ = ["Environment", "parse_inst_and_initsol", "run"]


class SetCoverSolver:
    def __init__(self, num_elements: int = -1):
        self.num_elements = num_elements
        self._solution = None
        self._instance = Instance()
        self._initialized = False

    def add_set(self, elements: list[int], cost: float) -> int:
        if cost < 0:
            raise ValueError("Costs must be non-negative")
        if not all(0 <= e < self.num_elements for e in elements):
            raise ValueError("Elements must be in range [0, num_elements)")
        self._instance.add(elements, cost)
        self._initialized = False
        return len(self._instance.costs) - 1

    def from_file(self, filename: str | Path, parser: str = "RAIL") -> None:
        env = Environment()
        env.time_limit = 10
        env.verbose = 2
        env.inst_path = str(filename)
        env.parser = parser
        fdata = parse_inst_and_initsol(env)
        self._instance = fdata.inst.copy()
        self._solution = fdata.init_sol.copy()
        print(self._solution)
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
        if not self._initialized:
            self._instance.rows.clear()
            self._instance.fill_rows_from_cols(self.num_elements)
            self._initialized = True
        init_sol = Solution() if self._solution is None else self._solution
        self._solution = run(env, self._instance, init_sol).copy()

    def get_solution(self) -> list[int] | None:
        """
        Return the indices of the selected sets in the solution.
        """
        if self._solution is None:
            return None
        return list(self._solution.idxs)

    def get_cost(self) -> float | None:
        """
        Return the cost of the solution.
        """
        if self._solution is None:
            return None
        return self._solution.cost

    def get_lower_bound(self) -> float:
        """
        Return a lower bound on the optimal solution.
        """
        if self._solution is None:
            return 0
        return self._solution.lower_bound
