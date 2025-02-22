<!--
SPDX-FileCopyrightText: 2024 Francesco Cavaliere <francescocava95@gmail.com>
SPDX-License-Identifier: MIT
-->

# AC-CFT
[![Language](https://img.shields.io/badge/C++-11-purple.svg)](https://en.cppreference.com/w/cpp/11)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Ubuntu CI](https://github.com/c4v4/cft/actions/workflows/c-cpp.yml/badge.svg?branch=main)](https://github.com/c4v4/cft/actions/workflows/c-cpp.yml)
[![codecov](https://codecov.io/gh/c4v4/cft/graph/badge.svg?token=2KKRX2KK7J)](https://codecov.io/gh/c4v4/cft)
[![Codacy Badge](https://app.codacy.com/project/badge/Grade/e1b326b8671444f3ad1d2c767a28a115)](https://app.codacy.com?utm_source=gh&utm_medium=referral&utm_content=&utm_campaign=Badge_grade)
[![REUSE status](https://api.reuse.software/badge/github.com/c4v4/cft)](https://api.reuse.software/info/github.com/c4v4/cft)


Implementation of the Caprara, Fischetti, and Toth algorithm for the [Set Covering problem](https://en.wikipedia.org/wiki/Set_cover_problem).

The project uses the C++11 standard. All the algorithmic components are placed in header files ([`src`](src/) folder) to simplify the integration into other projects.

## References
*Caprara, A., Fischetti, M., & Toth, P. (1999). A Heuristic Method for the Set Covering Problem. Operations Research, 47(5), 730–743. [doi:10.1287/opre.47.5.730](https://doi.org/10.1287/opre.47.5.730)*

## Building and Running

Configure the project and build it:

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
```

The binary will be located in `build/accft`.
You can run it with:

```bash
./build/accft -i instances/rail/rail507 -p RAIL
```

See `./build/accft --help` for the list of available command line arguments and their meaning.

## Tests and Coverage

To produce a debug build with tests enabled:

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Debug -DUNIT_TESTS=ON 
cmake --build build -j
```

Run the tests:

```bash
ctest --test-dir build -j
```

Use the following commands to generate code coverage statistics, note that you need to have [`lcov`](https://github.com/linux-test-project/lcov) installed.

```bash
mkdir -p coverage
lcov -c -d build -o coverage/tests_cov.info
lcov -e coverage/tests_cov.info cft/src/*/ -o coverage/tests_cov.info
```

Finally, to generate an HTML-report that can be viewed from your browser:

```bash
genhtml coverage/tests_cov.info --legend --output-directory=./coverage
```

You can check out the coverage statistics by opening `coverage/index.html`.

## Algorithm Structure
Here we'll briefly describe the key components of the algorithm. For a more in-depth and formal description you can refer to the original [paper](https://doi.org/10.1287/opre.47.5.730), or directly look at the code.

The CFT algorithm has a hierarchical structure, with each layer applying different column selection techniques to simplify the problem for the next step. Here's a breakdown of the main components, starting from the outermost layer:

- [`cft::run`](src/algorithms/Refinement.hpp): This function contains the full algorithm, it implements what is called "Refinement" in the original paper, which fixes part of the best solution found so far and then invokes the 3-phase procedure on the obtained subinstance.

- [`cft::ThreePhase`](src/algorithms/ThreePhase.hpp): This function, as the name suggests, involves three steps:
    - A subgradient step that finds near optimal multipliers to guide the search and also selects a small set of high quality columns (core-instance) to intensify and speed-up the search.
    - A heuristic phase that performs some other subgradient iterations to explore diversified multipliers and invokes the greedy procedure on each one of them to find good-quality solutions.
    - A column fixing step that selects and fixes some elements that have a good probability of being part of high-quality solutions, incrementally reducing the problem size.

- [`cft::Greedy`](src/greedy/Greedy.hpp): Fast greedy algorithm run during the heuristic phase of the 3-phase. It starts from a set of Lagrangian multipliers and then it iteratively selects columns based on their quality. At the end, it checks and removes redundant elements from the obtained solution (if any).

## Integration in other projects
The project comes in the form of an executable that reads an instance file and runs the full CFT algorithm. If you want to integrate the CFT into your own project, here we'll described some of it's main component that can be useful on their own.

### Instance
To start, a well formed instance is essential to run the algorithm correctly. The `Instance` struct is defined in [`Instance.hpp`](src/core/Instance.hpp):
```cpp
    struct Instance {
        SparseBinMat<ridx_t>             cols;
        std::vector<std::vector<cidx_t>> rows;
        std::vector<real_t>              costs;
    };
```

- `cols`: it contains a sparse "column-major" view of the constraint matrix, where each sparse-column in placed one after the other in a single `std::vector`, and a second vector contains the begin and end indexes of each column in the vector.

- `row`: the same could be done for storing rows, however, since the "row-major" view is used less frequently and since we assume that the number of columns is way larger than the number of rows, a vector of vectors of indexes is used instead, since it is simpler to manage and does not add a noticeable overhead.

- `costs`: a vector containing the cost for each column.

Here's an example of how to build a dummy instance:
```cpp
    auto inst = cft::Instance();

    // Insert the columns one at a time
    inst.cols.push_back({0, 1, 2, 3});
    inst.cols.push_back({1, 3, 0, 4});
    inst.cols.push_back({4, 1, 2});

    // Define columns costs (Note, every column must have a cost)
    inst.costs = {1.0, 2.0, 3.0};

    // Rows can be automatically filled from columns using this helper function
    cft::fill_rows_from_cols(inst.cols, 5, inst.rows);

    // Check that the defined instance is well formed (costly operation)
    CFT_IF_DEBUG(cft::col_and_rows_check(inst.cols, inst.rows));
```

### Full run
To run the full CFT algorithm you have to first declare an `Environment`, possibly changing the default parameters values, look at [`cft.hpp`](src/core/cft.hpp) for more details on the available parameters. Then you can call [`cft::run`](src/algorithms/Refinement.hpp) which, on completion, will return the best solution found.
```cpp
    auto env       = cft::Environment();
    env.time_limit = 10.0;  // Time limit in seconds
    env.verbose    = 2;     // Log verbosity level (from 0 to 5)
    env.timer.restart();    // NOTE: the timer is used also to test the time limit
    auto sol = cft::run(env, inst);
    fmt::print("CFT solution cost: {}\n", sol.cost);

```
### 3-Phase
If instead you are not interested in the outer-most column fixing (the "Refinement" step), you can call directly the 3-phase. Note that it is provided as a function object:
```cpp
    auto three_phase = cft::ThreePhase();
    auto result      = three_phase(env, inst);
    fmt::print("3-phase solution cost: {}, LB: {}\n", result.sol.cost, result.nofix_lb);
```
### Greedy
Finally, if you are only interested in a quick solution and do not care too much about its quality, you can ignore most of the "math-based" techniques used to guide the search, and directly run the greedy step like this:
```cpp
    auto num_rows      = cft::rsize(inst.rows);
    auto lagr_mult     = std::vector<cft::real_t>(num_rows, 0.0);
    auto reduced_costs = inst.costs;  // Zero multipliers implies red-costs = costs

    auto greedy = cft::Greedy();
    auto sol    = cft::Solution();
    sol.cost    = greedy(inst, lagr_mult, reduced_costs, sol.idxs);
    fmt::print("Greedy solution cost: {}\n", sol.cost);
```
In this case, the multipliers can be viewed as a set of weigths that measure row importance, so they can be manipulated to guide the greedy (which is exactly what it is done within the 3-phase).

## Benchmarks

Benchmarks have been run on ThinkPad P16v with an [`intel i7-13700H`](https://www.cpubenchmark.net/cpu.php?cpu=Intel+Core+i7-13700H) processor and 32GB of RAM on an Arch system, compiled with `g++ 13.2.1`.

We run the tests using the script located in [`benchmarks/run_all.sh`](benchmarks/run_all.sh) with the project compiled in `Release`.

We tested each instance with 10 different random seeds.
The runs were not subject to any time limit, since we used the termination criterion described in the original paper.

### Rail Instances

| Instance                               |   #Rows|    #Cols|  Best Sol |   Avg Sol | Avg Time(s) |
|:---                                    |    ---:|     ---:|       ---:|       ---:|         ---:|
| [`rail516`](instances/rail/rail516)    |    516 |   47311 |    182.00 |    182.00 |        2.10 |
| [`rail582`](instances/rail/rail582)    |    582 |   55515 |    211.00 |    211.00 |        1.02 |
| [`rail507`](instances/rail/rail507)    |    507 |   63009 |    174.00 |    175.00 |        3.77 |
| [`rail2586`](instances/rail/rail2586)  |   2586 |  920683 |    950.00 |    950.80 |       57.14 |
| [`rail4872`](instances/rail/rail4872)  |   4872 |  968672 |   1533.00 |   1534.60 |      160.09 |
| [`rail2536`](instances/rail/rail2536)  |   2536 | 1081841 |    692.00 |    694.00 |       85.69 |
| [`rail4284`](instances/rail/rail4284)  |   4284 | 1092610 |   1066.00 |   1067.50 |       91.58 |

_Note: for rail2586, better results can be obtained emphasizing multipliers quality._

The results for the other datasets can be found in the [`benchmarks`](benchmarks/) directory.


## Python Bindings

The project also provides Python bindings with a simplified interface for quick usage.
While you can also access much of the C++-interface directly, the primary purpose is to provide a point and shoot interface for the algorithm.
You can find more details in the separate [README](README.py.md).

## Coding Style

Regarding the source code, here you can find the general set of rules that we try to enforce. Since this is a project we work on in our free time, we haven't been afraid to experiment with simple rules and convention that we adjusted along the way when we felt something wasn't working for us.

We decided to stick with the C++11 standard for a couple of reasons. First, it keeps the code accessible to more people. Second, it helped us avoid the temptation of using fancy meta-programming features and abstractions introduced in recent standards, which can easily result in moving the focus from the algorithm itself to the _how_ the algorithm is implemented.

### External Dependencies
The only external dependecy that we have is [`fmtlib`](https://github.com/fmtlib/fmt), a printing/formatting library that is so much superior to `iostream` that it's a pain when we can't use it (luckily it got into the standard).
If you build the project with CMake, it should download and configure it automatically. Note, however, that you'll need an internet connection for that to work.


### User Vs Library Code

C++ is a hefty language. One of its main perks is its ability to define abstractions that can be general enough to serve a wide variety of use-cases (the STL is a clear example of that). However, a large part of the features that this language provides are redundant, unnecessary and simply add up to the overall complexity, which often makes code very hard to understand. 

For the algorithm implementation, we chose to limit ourselves to a _small_ set of language features, with the freedom of ignoring these restrinctions when doing so can really improve both the coding process and the understanding of the code. We place these helpers tools in the [`utils`](src/utils/) folder to keep them separated from the algorithm implementation. This approach creates two coding contexts:

- `user-code`: which is pretty much C code extended with a fairly small set of C++ features.
- `library-code`: which provides a carefully chosen set of abstractions.

### User Code

Along with the classic entities proper of the C programming language, we **define** other two types of "objects":

- `C-like structs`: (_structs_ in the following) simple aggregates of other objects (even C++ objects). There are not many rules about this kind of objects, they simply aim to be the _"sum of their types"_.
- `Function objects`: (improperly called _functor_ in the following) which are functions with an associated state, implemented as structs that define the `operator()` member function.

Other rules:

- We limit implicit casts as much as reasonably possible.
- No direct storage of resources that require special handling in constructors/destructors (e.g., pointers to allocated memory or file handles).
- No storage of references or pointers within either _structs_ or _functors_. They can be stored by other abstractions defined in _library code_.
- No `const` member variables.
- No explicit memory management in general, everything is wrapped within a proper abstraction.
- No (user-defined) copy/move constructors/assignment operators, no destructors.
- All objects must be default-constructible.
- _structs_ have only public member variables.
- _structs_ state validity is user's responsibility.
- _structs_ have no constructors or other user-defined member functions.
- _functors_ can have non-default constructors.
- _functors_ must always be in a valid state.
- _functors_ cannot have public member variables, they can only be used as a function abstraction.

To accommodate potential changes to the basic numeric types, we have introduced custom type-aliases and user-defined literals to express the corresponding numeric constants.

### Library Code

Not much to say here, library code tries to adhere to _user-code_ rules, but has the freedom to break some of them for the sake of avoiding bugs or making the abstraction more intuitive/easier to use. Factory methods are often used to have a hand-made CTAD. Templates are used but we avoid meta-programming when possible.

### Function Parameters

Looking around you might notice that most function parameters are annotated with tags. Although intuitive, here is their meaning:

- **`in`**: Input parameter, read-only.
- **`out`**: Ouput parameter, previous state is ignored and replaced with the new state.
- **`inout`**: Input/output parameter. Both previous and new states are relevant.
- **`cache`**: Cache object to avoid costly operations (usually memory allocations). Input and output states are ignored.

### Custom Types

This project offers flexibility in choosing the numeric types used for column indexes, row indexes, and real values. We provide type-aliases (`cidx_t`, `ridx_t`, `real_t`) defined in [`src/core/cft.hpp`](src/core/cft.hpp) that you can customize by defining the corresponding macros (`CFT_CIDX_TYPE`, `CFT_RIDX_TYPE`, `CFT_REAL_TYPE`). This allows you to easily switch between native integer/floating-point types depending on your needs.

Beyond native types, we also support defining custom types through a simple interface. You can find an example implementation in [`test/custom_types_unittests.cpp`](test/custom_types_unittests.cpp). 
For instance, you can use this interface to:

- _Enforce Checks_: Implement custom checks for every mathematical operation performed.
- _Control Type Conversions_: Restrict implicit casts, e.g., to avoid mixing row and column variables.
- _Introduce Specialized Types_: Utilize alternative number representations like fixed-point types for specific use cases.

This approach tries to strike a good a balance between ease of use for common numeric types and the ability to tailor the library to your specific needs.
