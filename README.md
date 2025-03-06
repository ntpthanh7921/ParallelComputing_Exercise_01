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
│   ├── mutex
│   └── python_data_collection
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

- `implementation`: contains implementation for sync primitives
- `performance`: contains test for performance/scalability for sync primitives
- `validation`: contains test for validation for sync primitives

## Tool

Use clang-format if possible.

Makefile is provided to quick run and test the code. Run `make help` for more information.

## Reference

Read [Reference.md](Reference.md)
