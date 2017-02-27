# Tests

Tests are located in the `/test` folder

### Test Runner

The main test runner is: `runtests.sh`

```cc
#for i in 01 02 03 04 05 06 09 10 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37; do
for i in test/00/*.sh; do
    if [ $i = "test/00/t0008a.sh" ]; then continue; fi # TODO schema 0 support
    # rendering to GIFs is too unstable, so diable on the Travis platform
    if [ "$TRAVIS" = 1 -a $i = test/00/t0035a.sh ]; then continue; fi
    sh $i
  status=$?
  if [ $status -ne 0 ]; then
      let $[t++]
      echo "$i exited with nonzero code $status"
  fi
done
```

The runner runs through all the shell scripts in `test/00` and executes each one.
Most of the current tests are GUI tests using the *Tcl*.

### Test Coverage

`test/run-tcl-cov.sh` can be used to generate a test coverage report `minsky.cov`

## Unit tests

The main unit tests are:

- `testVariable` - test variables
- `testDerivative` - test derivative calculations
- `testMinsky` test full diagram of connected nodes

## UI unit tests

The main unit tests are:

- `testGroup`
- `testGeometry`

## IO/Data tests

- `testDatabase` - SQL database tests

