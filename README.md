# AC-CFT

[![CMake on multiple platforms](https://github.com/falcopt/scp/actions/workflows/c-cpp.yml/badge.svg)](https://github.com/falcopt/scp/actions/workflows/c-cpp.yml)

Accorsi Luca and Cavaliere Francesco implementation of the CFT algorithm for the Set Covering problem.

## Build and run unit tests
To build unit tests
```
cmake .. -DUNITTESTS=1
make -j
```
To run the tests
```
./tests
```
To get code coverage statistics
```
gcovr --html-details build/coverage.html
```
from the main project directory (not from the `build` dir!).
Then checkout `build/coverage.html`.


## How to contribute
Create a new branch
```
git checkout -b new-feature
```
Edit the code in the new branch and push it
```
git add --all
git commit -m "Message"
git push -u origin new-feature
```
Create a pull request on the github UI and add a reviewer.

To include suggested changes
```
git commit --amend -a
git push -u origin new-feature --force
```

## References
*Caprara, A., Fischetti, M., & Toth, P. (1999). A Heuristic Method for the Set Covering Problem. Operations Research, 47(5), 730–743. [doi:10.1287/opre.47.5.730](https://doi.org/10.1287/opre.47.5.730)*


## Coding Style

Here you can find the general set of rules that we try to enforce, in case you are interested in reading the source code. Being this a stand-alone project at which we work in our spare time, we took the freedom to experiment a little with simple conventions, changing from time to time when we noticed that something didn't fit our needs.

First of all, we constrained ourselves to the C++11 standard. The main reason for this choice has been to potentially reach a wider audience. This choice was also made to limit the use of fancy meta-programming features and abstractions which can easily result in moving the focus from the algorithm itself to the _how_ the algorithm is implemented.

### User Vs Library Code

C++ is a hefty language. As we just said, one of the main perks we see in this language is its ability to define abstractions that can be general enough to serve a wide variety of use-cases (the STL is a clear example of that). However, most of the features that this language provides are redundant, unnecessary and simply add up to the overall complexity of this language, which often makes code very hard to understand. For this reason, for the algorithm implementation, we chose to limit ourselves to a _small_ set of language features. When an abstraction can really streamline both the coding process and the understanding of the code, we allow ourselves to use and write simple and carefully chosen abstractions. They are placed in the `core` folder. This division creates two coding contexts:

- `user-code`: which is pretty much C code extended with a fairly small set of C++ features that we think really add value.
- `library-code`: which, as the STL, provide a carefully chosen set of abstractions we find useful.

We do not report here classic rules like upper/lower-case naming conventions, trailing return type or not, etc., just open the code and you'll see what we like, in the end, these things are mostly a matter of taste, so we avoid spending time justifying these kinds of choices.

### User Code

Along with the classic entities proper of the C programming language, we **define** other two types of "objects":

- `C-like structs`: (_structs_ in the following) simple aggregates of other objects (even C++ objects). There are not many rules about this kind of objects, they simply aim to be the _"sum of their types"_.
- `Function objects`: (improperly called _functor_ in the following) which simply are functions with an associated state, implemented as structs that define the `operator()` member function.

Other rules:

- No references or pointers within either _structs_ or _functors_, clearly they can be stored by other abstractions defined in _library code_, like `std::vector`.
- _structs_ have only public member variables.
- _structs_ can be initialized by factories (or manually), no constructors or other user-defined member functions.
- _functors_ cannot have public member variables, they can only be accessed as a function abstraction.
- _functors_ can have non-default constructors.
- No (user-defined) copy/move constructors/assignment operators, no destructors.

### Library Code

Not much to say here, library code tries to adhere to _user-code_ rules, but has the freedom to break some of them for the sake of avoiding bugs or making the abstraction more intuitive/easier to use. Factory methods are often used to have a hand-made CTAD (which was not present in C++11). Templates are used but the meta-programming is kept at the minimum.
