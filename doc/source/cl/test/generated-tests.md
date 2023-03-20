# GeneratedTests test suite

A test suite based around LLVM's lit tool for executing tests which are
automatically generated from OpenCL kernels, and summarizing their results.

## Generating tests

Tests in this tests suite should be generated by running
`source/cl/test/scripts/create_tests.sh` on a set of kernels.

## Building

While the lit tests themselves do not need to be compiled, their configuration
files are configured through CMake in order to set up correct paths and other
parameters. The tests are build together with the other test suites, but they
can also be built individually though the `GeneratedTests` target.

In order for the tests to be built, the `FileCheck` and `lit` tools need to be
in a path (eg the `PATH`) that CMake can find them. If CMake is not able to find
them, the user needs to provide the paths manually, through the
`CA_LLVM_{FILECHECK,LIT}_PATH` variables. If these files are not found, **all**
the lit tests will be disabled.

* The `FileCheck` tool is built by LLVM. For `FileCheck` to be installed, the
  `LLVM_INSTALL_UTILS` option needs to be set when running the CMake for
  building LLVM. Alternatively, the tool can probably be found in the `bin`
  folder in the LLVM build directory.
* The `lit` tool is a Python package and can be installed through pip or any
  other Python package manager. The `llvm-lit` tool is used internally by LLVM
  and it **will not** be installed alongside `FileCheck`. It can be found in the
  same build directory as `FileCheck` though.

## Executing

To run Codeplay's lit tests simply invoke the `lit` tool from the command line,
passing in either a directory of tests or an individual test to run. To see more
information about test failures you can pass the `--verbose` flag to `lit`. Or
to debug the test suite itself `--debug` can be set.

Sample usage:

```
lit GeneratedTests/ --xunit-xml-output=lit_junit.xml
```

Alternative, you can use the `llvm-lit` wrapper, which can also generate JUnit
XML output with the `--xunit-xml-output` option.

Sample usage:

```
llvm-lit GeneratedTests/ --xunit-xml-output=lit_junit.xml
```

Note that the `GeneratedTests` directory is the one found in the build directory
and not the one in the source directory. Attempting to run the tests in the
source directory will fail as the paths of the files required will not have been
set.