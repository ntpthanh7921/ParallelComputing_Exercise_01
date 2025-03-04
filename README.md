# Parallel Computing Exercise 01 - sync primitives

## Structure

The folder structure of this project is as follows:

```
.
├── implementation
│   ├── barrier.c
│   ├── barrier.h
│   ├── counter.c
│   ├── counter.h
│   ├── mutex.c
│   ├── mutex.h
│   ├── test.cpp
│   └── test.h
├── performance
│   ├── performance_hello_world_again.cpp
│   └── performance_hello_world.cpp
├── validation
│   └── validation_hello_world.cpp
├── LICENSE
├── Makefile
└── README
```

Where:

- `implementation`: contains implementation for sync primitives
- `performance`: contains test for performance/scalability for sync primitives
- `validation`: contains test for validation for sync primitives

## Tool

Use clang-format if possible.

Makefile is provided to quick run and test the code. Run `make help` for more information.
