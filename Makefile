# Makefile for C++ Synchronization Primitives Project

# Compiler settings
CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++20 -I. -I./implementation -pthread -fopenmp
LDFLAGS = -pthread -fopenmp

# Find all source files
VALIDATION_SRCS = $(shell find validation -name "*.cpp")
PERF_SRCS = $(shell find performance -name "*.cpp")
IMPL_SRCS = $(shell find implementation -name "*.cpp")

# Find all source and header files for formatting
FORMAT_SRCS = $(shell find . -name "*.cpp" -or -name "*.hpp" -or -name "*.h")

# Find all implementation sources and headers
IMPL_HDRS = $(shell find implementation -name "*.hpp" -or -name "*.h")
IMPL_OBJS = $(IMPL_SRCS:.cpp=.o)

# Find all header files
HDRS = $(shell find . -name "*.hpp" -or -name "*.h")

# Generate object file lists and executables
VALIDATION_OBJS = $(VALIDATION_SRCS:.cpp=.o)
PERF_OBJS = $(PERF_SRCS:.cpp=.o)
VALIDATION_EXECS = $(VALIDATION_SRCS:.cpp=)
PERF_EXECS = $(PERF_SRCS:.cpp=)

# Default target
all: $(VALIDATION_EXECS) $(PERF_EXECS)

# Generic rule for validation executables
validation/%: validation/%.o $(IMPL_OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $< $(IMPL_OBJS) $(LDFLAGS)
	chmod +x $@

# Generic rule for performance executables
performance/%: performance/%.o $(IMPL_OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $< $(IMPL_OBJS) $(LDFLAGS)
	chmod +x $@

# Compile implementation source files
implementation/%.o: implementation/%.cpp $(IMPL_HDRS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Compile test source files
%.o: %.cpp $(HDRS) $(IMPL_HDRS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Validation target - run all validation tests
validate: $(VALIDATION_EXECS)
	@for test in $(VALIDATION_EXECS); do \
		echo "Running validation test: $$test"; \
		./$$test; \
	done

# Performance test target - run all performance tests
performance: $(PERF_EXECS)
	@for test in $(PERF_EXECS); do \
		echo "Running performance test: $$test"; \
		./$$test; \
	done

# Clang-format target
format:
	@if command -v clang-format >/dev/null 2>&1; then \
		echo "Running clang-format on all source files..."; \
		clang-format -i -style=file $(FORMAT_SRCS); \
		echo "Formatting complete."; \
	else \
		echo "Error: clang-format not found. Please install clang-format."; \
		exit 1; \
	fi

# Clang-format check target (doesn't modify files)
format-check:
	@if command -v clang-format >/dev/null 2>&1; then \
		echo "Checking formatting of source files..."; \
		clang-format -n -Werror -style=file $(FORMAT_SRCS); \
		if [ $$? -eq 0 ]; then \
			echo "Formatting check passed."; \
		else \
			echo "Formatting check failed. Run 'make format' to fix."; \
			exit 1; \
		fi \
	else \
		echo "Error: clang-format not found. Please install clang-format."; \
		exit 1; \
	fi

# Clean up
clean:
	find . -name "*.o" -delete
	rm -f $(VALIDATION_EXECS) $(PERF_EXECS)

# Help target
help:
	@echo "Targets:"
	@echo "  all          : Build all test executables (default)"
	@echo "  validate     : Build and run ALL validation tests"
	@echo "  performance  : Build and run ALL performance tests"
	@echo "  format       : Run clang-format on all source files"
	@echo "  format-check : Check formatting without modifying files"
	@echo "  clean        : Remove all generated files"
	@echo "  help         : Show this help message"

# Phony targets
.PHONY: all clean validate performance format format-check help
