# Parallel Computing Exercise 01 - sync primitives

## Structure

The folder structure of this project is as follows:

```
.
├── implementation
│   ├── barrier.hpp
│   ├── counter.hpp
│   ├── cpu_pause.hpp
│   ├── mutex_competitive.hpp
│   └── mutex_mcs.hpp
├── performance
│   ├── barrier
│   ├── counter
│   └── mutex
├── python_data_collection
│   ├── phuocthanh_latitude
│   ├── mutex_test_many_params_with_log.py
│   └── run_mutex_tests.sh
├── validation
│   ├── barrier
│   ├── counter
│   └── mutex
├── LICENSE
├── Makefile
├── README.md
└── Reference.md
```

Where:

- `implementation`: contains implementation for sync primitives.

- `performance`: contains test for performance/scalability for sync primitives.
Each cpp file is assumed to be run as an independent test.

- `validation`: contains test for validation for sync primitives. Each cpp file
is assumed to be run as an independent test.

- `python_data_collection`: contains Python script for performance testing with
various combination of parameters. Also stores test results on various platform

## Quick-start

1. Run `make all`

2. Validate sync primitives by running `make validate`

3. Performance test sync primitives by running `make performance`. This will
   only run each test once with default parameter values.

4. To running performance test with parameter tuning (basically means trying
   many combinations). See the [next section](#how-to-run-exhaustive-performance-test).

## How to run exhaustive performance test

For this, we must go into the `python_data_collection` directory.

### Spinlock mutex

To test spinlock mutex, make sure that your environment have access to `python`
(see [Tool](#tool) on required libraries).

1. Make script executable: `chmod +x run_mutex_tests.sh`
2. Run it with: `./run_mutex_tests.sh /path/to/your/executables`

Replace path to your executables to the directory containing the performance
test executables. The script will find all executable within the directory and
run test on them.

Result will be stored in new directories named after the executable. There will
be a logfile, a detailed report file and a CSV file. For further analysis, you
can concentrate on the CSV file.

### Barrier

Similar to testing [Spinlock mutex](#spinlock-mutex)

1. Make script executable: `chmod +x run_barrier_tests.sh`
2. Run it with: `./run_barrier_tests.sh /path/to/your/executables`

Replace path to your executables to the directory containing the performance
test executables. The script will find all executable within the directory and
run test on them.

Result will be stored in new directories named after the executable. There will
be a logfile, a detailed report file and a CSV file. For further analysis, you
can concentrate on the CSV file.

### Counter

(to be added)

## Tool

### `clang-format`

`clang-format` is used to format the code within this project. The style
definition is `.clang-format` file (not shown on directory structure).

### `make`

`make` is use for:

- compilation of validation test and performance test binary
- running validation test
- running simple simple performance test with default parameter values
- check formatting of code using `clang-format`

The Makefile is have some weird dependencies such that it recompile more than
needed. But for the purpose of this project, this behavior does not matter.

Run `make help` for more information.

### `python`

Python is used for conveniently running performance test with many combinations
of parameter and logging the result. The needed package listing are:

- `pandas`

### `bash`

Also used for running and logging performance test. Why don't I just use only
Python? I like to use bash for the thing that I use it for in this project,
that's all.

## Reference

For reference material (e.g. design of sync primitives), see [Reference.md](Reference.md)
