# SPDX-FileCopyrightText: 2025 Dominik Krupke <krupked@gmail.com>
# SPDX-License-Identifier: MIT

import pycft
import pytest


def test_simple():
    solver = pycft.SetCoverSolver()
    # solver.from_file("instances/rail/rail507", "RAIL")
    solver.add_set([0, 1, 2, 3, 4, 5, 6, 7, 8, 9], cost=10)
    solver.add_set([0, 1, 2, 3, 4, 5], cost=5)
    solver.add_set([0, 1, 2, 3, 4], cost=4)
    solver.add_set([6, 7, 8, 9], cost=4)
    solver.solve()
    solution = solver.get_solution()
    assert solution is not None, "No solution found"
    assert 1 in solution, "Element 1 not in solution"
    assert 3 in solution, "Element 3 not in solution"
    assert solver.get_cost() == 9, "Objective value is not 9"
    assert solver.get_lower_bound() >= 8.9, "Lower bound is not 9.0"


def test_simple_infeasible():
    solver = pycft.SetCoverSolver()
    # solver.from_file("instances/rail/rail507", "RAIL")
    solver.add_set([0, 1, 2, 3, 4, 5, 6, 8, 9], cost=10)
    solver.add_set([0, 1, 2, 3, 4, 5], cost=5)
    solver.add_set([0, 1, 2, 3, 4], cost=4)
    solver.add_set([6, 8, 9], cost=4)
    with pytest.raises(RuntimeError):
        solver.solve()


def test_error_on_negative_cost():
    solver = pycft.SetCoverSolver()
    with pytest.raises(ValueError):
        solver.add_set([0, 1, 2, 3, 4, 5, 6, 8, 9], cost=-10)


def test_error_on_negative_element():
    solver = pycft.SetCoverSolver()
    with pytest.raises(ValueError):
        solver.add_set([-1, 1, 2, 3, 4, 5, 6, 8, 9], cost=10)


def test_simple_incremental():
    solver = pycft.SetCoverSolver()
    # solver.from_file("instances/rail/rail507", "RAIL")
    solver.add_set([0, 1, 2, 3, 4, 5, 6, 7, 8, 9], cost=10)
    solver.add_set([0, 1, 2, 3, 4, 5], cost=5)
    solver.add_set([0, 1, 2, 3, 4], cost=4)
    solver.add_set([6, 7, 8, 9], cost=4)
    solver.solve()
    solution = solver.get_solution()
    assert solution is not None, "No solution found"
    assert 1 in solution, "Element 1 not in solution"
    assert 3 in solution, "Element 3 not in solution"
    assert solver.get_cost() == 9, "Objective value is not 9"
    assert solver.get_lower_bound() >= 8.9, "Lower bound is not 9.0"
    solver.add_set([0, 1, 2, 3, 4, 5, 6, 7, 8, 9], cost=5)
    solver.solve()
    solution = solver.get_solution()
    assert solver.get_cost() == 5, "Objective value is not 5"


def test_simple_incremental_new_element():
    solver = pycft.SetCoverSolver()
    # solver.from_file("instances/rail/rail507", "RAIL")
    solver.add_set([0, 1, 2, 3, 4, 5, 6, 7, 8, 9], cost=10)
    solver.add_set([0, 1, 2, 3, 4, 5], cost=5)
    solver.add_set([0, 1, 2, 3, 4], cost=4)
    solver.add_set([6, 7, 8, 9], cost=4)
    solver.solve()
    solution = solver.get_solution()
    assert solution is not None, "No solution found"
    assert 1 in solution, "Element 1 not in solution"
    assert 3 in solution, "Element 3 not in solution"
    assert solver.get_cost() == 9, "Objective value is not 9"
    assert solver.get_lower_bound() >= 8.9, "Lower bound is not 9.0"
    solver.add_set([0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10], cost=15)
    solver.solve()
    solution = solver.get_solution()
    assert solver.get_cost() == 15, "Objective value is not 15"
