# Tests

Tests are located in the `/test/00` folder. Each test is a shell script that returns 0 on success and non-zero on failure, with file name ending in .sh. Test scripts should clean up any temporary directories created by them.

### Test Runner

- To run the tests, type "make sure" at the toplevel directory. This ensures all necessary executables are built prior to runnning the test scripts.

- Individual tests can be run by running specific scripts from the test/00 directory.

### Test Coverage

`test/run-tcl-cov.sh` can be used to generate a test coverage report `minsky.cov`

## Unit tests

One of the test scripts runs the `test/unittests` executable. This executable has tests written using the the UnitTest++ framework, which is typically good for writing low level tests of individual classes.

- `unittests -l` will list available tests
- `unittests <regexpPattern>` will run all tests matching the regexp. Gives a way of running a selected subset of tests
- without any argument, unittests runs all tests, ie is equivalent to `unittests .*`

## Integration tests

The minsky executable can run in batch mode by passing a TCL script on the command line.
- The GUI environment is not instantiated until the TCL script has completed. Therefore, and exit at the end of the script prevent the GUI environment being instantiated
- To run commands against the GUI, define a procedure afterMinskyStarted
- There is a TCL assert command available in test/assert.tcl.
   - assert expr comment
       if braces are placed around the expression, it is executed as part of the context of the assert proc, which doesn't have access to global variables. Instead, you can enclose the expression in quotes, and use the comment field to indicate what assertion failed.

