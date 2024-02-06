# AC-CFT
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
