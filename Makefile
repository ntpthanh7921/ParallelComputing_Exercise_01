# Makefile for C++ Synchronization Primitives Project

# Compiler settings
CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17 -I. -I./implementation
LDFLAGS = -pthread -fopenmp

# Find all source files
VALIDATION_SRCS = $(shell find validation -name "*.cpp")
PERF_SRCS = $(shell find performance -name "*.cpp")

# Find all implementation sources and headers
IMPL_SRCS = $(shell find implementation -name "*.cpp")
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
	@echo "  clean        : Remove all generated files"
	@echo "  help         : Show this help message"

# Phony targets
.PHONY: all clean validate performance help
