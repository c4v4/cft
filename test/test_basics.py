import pyaccft


def test_simple():
    solver = pyaccft.SetCoverSolver(10)
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
