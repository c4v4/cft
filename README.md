<!--
SPDX-FileCopyrightText: 2024 Francesco Cavaliere <francescocava95@gmail.com>
SPDX-License-Identifier: MIT
-->

# AC-CFT

[![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg)](https://www.gnu.org/licenses/gpl-3.0)
[![Ubuntu CI](https://github.com/c4v4/cft/actions/workflows/c-cpp.yml/badge.svg?branch=main)](https://github.com/c4v4/cft/actions/workflows/c-cpp.yml)
[![codecov](https://codecov.io/gh/c4v4/cft/graph/badge.svg?token=2KKRX2KK7J)](https://codecov.io/gh/c4v4/cft)
[![Codacy Badge](https://app.codacy.com/project/badge/Grade/e1b326b8671444f3ad1d2c767a28a115)](https://app.codacy.com?utm_source=gh&utm_medium=referral&utm_content=&utm_campaign=Badge_grade)

Implementation of the Caprara, Fischetti, and Toth algorithm for the [Set Covering problem](https://en.wikipedia.org/wiki/Set_cover_problem).

## References
*Caprara, A., Fischetti, M., & Toth, P. (1999). A Heuristic Method for the Set Covering Problem. Operations Research, 47(5), 730–743. [doi:10.1287/opre.47.5.730](https://doi.org/10.1287/opre.47.5.730)*

## Building and Running the Project

Configure the project for a release build and build it:

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
```

The binary will be located at `build/accft`.
You can run it with:

```bash
./build/accft -i instances/rail/rail507 -p RAIL -t 30 
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
ctest --test-dir build
```

Use the following commands to generate code coverage statistics, note that you need to have [`lcov`](https://github.com/linux-test-project/lcov) installed.

```bash
mkdir -p coverage
lcov -c -d build -o coverage/tests_cov.info
lcov -r coverage/tests_cov.info build test /usr/include/ -o tests_cov.info
```

Finally, to generate an HTML-report that can be viewed from your browser:

```bash
genhtml coverage/tests_cov.info --legend --output-directory=./coverage
```

You can check out the coverage statistics by opening `coverage/index.html`.

## Benchmarks
_Coming soon..._

## Coding Style

In case you might be interested in reading the source code, here you can find the general set of rules that we try to enforce. Being this a stand-alone project at which we work in our spare time, we took the freedom to experiment a little with simple conventions, changing from time to time when we noticed that something didn't fit our needs.

First of all, we constrained ourselves to the C++11 standard. The main reason for this choice has been to potentially reach a wider audience. This choice was also made to limit the use of fancy meta-programming features and abstractions which can easily result in moving the focus from the algorithm itself to the _how_ the algorithm is implemented.

### User Vs Library Code

C++ is a hefty language. One of its main perks is its ability to define abstractions that can be general enough to serve a wide variety of use-cases (the STL is a clear example of that). However, most of the features that this language provides are redundant, unnecessary and simply add up to the overall complexity of the language, which often makes code very hard to understand. For this reason, for the algorithm implementation, we chose to limit ourselves to a _small_ set of language features. When an abstraction can really streamline both the coding process and the understanding of the code, we allow ourselves to use and write simple and carefully chosen abstractions. They are placed in the `utils` folder. This division creates two coding contexts:

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


### Library Code

Not much to say here, library code tries to adhere to _user-code_ rules, but has the freedom to break some of them for the sake of avoiding bugs or making the abstraction more intuitive/easier to use. Factory methods are often used to have a hand-made CTAD (which was not present in C++11). Templates are used but the meta-programming is kept at a low level.

### Function Parameters

Looking around you might notice that most function parameters are annotated with a tag. Although intuitive, here is the meaning:

- **`in`**: Input parameter, read-only.
- **`out`**: Ouput parameter, previous state is ignored and replaced with the new state.
- **`inout`**: Input/output parameter. Both previous and new states are relevant.
- **`cache`**: Cache object to avoid costly operations (usually memory allocations). Input and output states are ignored.

### Custom Types

This project offers flexibility in choosing the numeric types used for column indexes, row indexes, and real values. We provide aliases (`cidx_t`, `ridx_t`, `real_t`) defined in [`src/core/cft.hpp`](src/core/cft.hpp) that you can customize by defining the corresponding macros (`CFT_CIDX_TYPE`, `CFT_RIDX_TYPE`, `CFT_REAL_TYPE`). This allows you to easily switch between native integer/floating-point types depending on your needs.

Beyond basic types, we also support defining custom types through a simple interface. You can find an example implementation in [`test/custom_types_unittests.cpp`](test/custom_type_unittests.cpp). 
For instance, you can use this interface to:

- Enforce Checks: Implement custom checks for every mathematical operation performed on your data.
- Control Type Conversions: Restrict implicit casts, e.g., to avoid mixing row and column variables.
- Introduce Specialized Types: Utilize alternative number representations like fixed-point types for specific use cases.

This approach tries to strike a good a balance between ease of use for common numeric types and the ability to tailor the library to your specific requirements.
