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

The project is implemented using the C++11 standars. All its algorithmic components are placed into header files to simplify the integration into other projects.

## References
*Caprara, A., Fischetti, M., & Toth, P. (1999). A Heuristic Method for the Set Covering Problem. Operations Research, 47(5), 730–743. [doi:10.1287/opre.47.5.730](https://doi.org/10.1287/opre.47.5.730)*

## Building and Running the Project

Configure the project and build it:

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
```

The binary will be located at `build/accft`.
You can run it with:

```bash
./build/accft -i instances/rail/rail507 -p RAIL
```

See `./build/accft --help` for the list of available parameters and their meaning.

## Tests and Coverage

To build the project with unit tests in debug mode:

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

## Benchmarks
Benchmark results were obtained using a ThinkPad P16v with an [Intel i7-13700H](https://www.cpubenchmark.net/cpu.php?cpu=Intel+Core+i7-13700H) processor and 32GB of RAM on an Arch system.

We run the tests using the script located in [`benchmarks/run_all.sh`](benchmarks/run_all.sh) with the project compiled in `Release` mode.

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

## Coding Style

In case you might be interested in reading the source code, here you can find the general set of rules that we try to enforce. As this is a stand-alone project at which we work in our spare time, we took the freedom to experiment a little with simple conventions, changing from time to time when we noticed that something didn't fit our needs.

First of all, we constrained ourselves to the C++11 standard. The main reason is to potentially reach a wider audience. We also made this choice to limit the use of fancy meta-programming features and abstractions introduced in recent standards, which can easily result in moving the focus from the algorithm itself to the _how_ the algorithm is implemented.

### User Vs Library Code

C++ is a hefty language. One of its main perks is its ability to define abstractions that can be general enough to serve a wide variety of use-cases (the STL is a clear example of that). However, a large part of the features that this language provides are redundant, unnecessary and simply add up to the overall complexity of the language, which often makes code very hard to understand. For this reason, for the algorithm implementation, we chose to limit ourselves to a _small_ set of language features. When an abstraction can really streamline both the coding process and the understanding of the code, we use and write simple and carefully chosen abstractions. They are placed in the `utils` folder. This approach creates two coding contexts:

- `user-code`: which is pretty much C code extended with a fairly small set of C++ features that we think really add value.
- `library-code`: which provide a carefully chosen set of abstractions we find useful.

We do not report here classic rules like upper/lower-case naming conventions, trailing return type or not, etc., just open the source-files and you'll see what we like, at the end, these things are mostly a matter of taste, so we avoid spending time justifying these kinds of choices.

### User Code

Along with the classic entities proper of the C programming language, we **define** other two types of "objects":

- `C-like structs`: (_structs_ in the following) simple aggregates of other objects (even C++ objects). There are not many rules about this kind of objects, they simply aim to be the _"sum of their types"_.
- `Function objects`: (improperly called _functor_ in the following) which simply are functions with an associated state, implemented as structs that define the `operator()` member function.

Other rules:

- Limit implicit casts as much as reasonably possible.
- No storage of references or pointers within either _structs_ or _functors_, clearly they can be stored by other abstractions defined in _library code_.
- No direct storage of resources that require special handling (e.g., pointers to allocated memory or file handles).
- No `const` member variables.
- No explicit memory management in general, everything is wrapped within a proper abstraction.
- No (user-defined) copy/move constructors/assignment operators, no destructors.
- Entities must always be default-constructible.
- _structs_ have only public member variables.
- _structs_ state validity is user's responsibility.
- _structs_ can be initialized by factories (or manually), no constructors or other user-defined member functions.
- _functors_ can have non-default constructors.
- _functors_ must always be in a valid state.
- _functors_ cannot have public member variables, they can only be accessed as a function abstraction.

Finally, to accommodate potential changes to the basic numeric types, we have introduced custom aliases, and, to minimize the use of implicit casts, we provide user-defined literals for defining numeric constants.

### Library Code

Not much to say here, library code tries to adhere to _user-code_ rules, but has the freedom to break some of them for the sake of avoiding bugs or making the abstraction more intuitive/easier to use. Factory methods are often used to have a hand-made CTAD. Templates are used but the meta-programming is kept low.

### Function Parameters

Looking around you might notice that most function parameters are annotated with tags. Although intuitive, here is their meaning:

- **`in`**: Input parameter, read-only.
- **`out`**: Ouput parameter, previous state is ignored and replaced with the new state.
- **`inout`**: Input/output parameter. Both previous and new states are relevant.
- **`cache`**: Cache object to avoid costly operations (usually memory allocations). Input and output states are ignored.

### Custom Types

This project offers flexibility in choosing the numeric types used for column indexes, row indexes, and real values. We provide aliases (`cidx_t`, `ridx_t`, `real_t`) defined in [`src/core/cft.hpp`](src/core/cft.hpp) that you can customize by defining the corresponding macros (`CFT_CIDX_TYPE`, `CFT_RIDX_TYPE`, `CFT_REAL_TYPE`). This allows you to easily switch between native integer/floating-point types depending on your needs.

Beyond native types, we also support defining custom types through a simple interface. You can find an example implementation in [`test/custom_types_unittests.cpp`](test/custom_types_unittests.cpp). 
For instance, you can use this interface to:

- _Enforce Checks_: Implement custom checks for every mathematical operation performed.
- _Control Type Conversions_: Restrict implicit casts, e.g., to avoid mixing row and column variables.
- _Introduce Specialized Types_: Utilize alternative number representations like fixed-point types for specific use cases.

This approach tries to strike a good a balance between ease of use for common numeric types and the ability to tailor the library to your specific requirements.
