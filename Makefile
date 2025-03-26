# Makefile for C++ Synchronization Primitives Project

# Compiler settings
CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++20 -I. -I./implementation -pthread -fopenmp
LDFLAGS = -pthread -fopenmp

# Build directory
BUILD_DIR = build

# Find all source files
VALIDATION_SRCS = $(shell find validation -name "*.cpp")
PERF_SRCS = $(shell find performance -name "*.cpp")
IMPL_SRCS = $(shell find implementation -name "*.cpp")

# Find all source and header files for formatting, excluding python_data_collection
FORMAT_SRCS = $(shell find . -not -path "./python_data_collection*" -name "*.cpp" -or -name "*.hpp" -or -name "*.h" | grep -v "python_data_collection")

# Find all implementation headers
IMPL_HDRS = $(shell find implementation -name "*.hpp" -or -name "*.h")
IMPL_OBJS = $(patsubst implementation/%.cpp,$(BUILD_DIR)/implementation/%.o,$(IMPL_SRCS))

# Find all header files
HDRS = $(shell find . -name "*.hpp" -or -name "*.h")

# Generate build paths for executables and objects
VALIDATION_OBJS = $(patsubst validation/%.cpp,$(BUILD_DIR)/validation/%.o,$(VALIDATION_SRCS))
PERF_OBJS = $(patsubst performance/%.cpp,$(BUILD_DIR)/performance/%.o,$(PERF_SRCS))
VALIDATION_EXECS = $(patsubst validation/%.cpp,$(BUILD_DIR)/validation/%,$(VALIDATION_SRCS))
PERF_EXECS = $(patsubst performance/%.cpp,$(BUILD_DIR)/performance/%,$(PERF_SRCS))

# Default target - just build, don't run
all: build-all

# Build all executables
build-all: build-validation build-performance

# Ensure build directories exist
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Create build subdirectories for validation tests
define make-validation-dirs
$(1): | $(BUILD_DIR)
	mkdir -p $(1)
endef

VALIDATION_DIRS = $(sort $(dir $(VALIDATION_OBJS) $(VALIDATION_EXECS)))
$(foreach d,$(VALIDATION_DIRS),$(eval $(call make-validation-dirs,$(d))))

# Create build subdirectories for performance tests
define make-performance-dirs
$(1): | $(BUILD_DIR)
	mkdir -p $(1)
endef

PERFORMANCE_DIRS = $(sort $(dir $(PERF_OBJS) $(PERF_EXECS)))
$(foreach d,$(PERFORMANCE_DIRS),$(eval $(call make-performance-dirs,$(d))))

# Create build subdirectories for implementation
define make-implementation-dirs
$(1): | $(BUILD_DIR)
	mkdir -p $(1)
endef

IMPLEMENTATION_DIRS = $(sort $(dir $(IMPL_OBJS)))
$(foreach d,$(IMPLEMENTATION_DIRS),$(eval $(call make-implementation-dirs,$(d))))

# Build validation executables
build-validation: $(VALIDATION_EXECS)

# Build performance executables
build-performance: $(PERF_EXECS)

# Generic rule for validation executables
$(BUILD_DIR)/validation/%: $(BUILD_DIR)/validation/%.o $(IMPL_OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $< $(IMPL_OBJS) $(LDFLAGS)
	chmod +x $@

# Generic rule for performance executables
$(BUILD_DIR)/performance/%: $(BUILD_DIR)/performance/%.o $(IMPL_OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $< $(IMPL_OBJS) $(LDFLAGS)
	chmod +x $@

# Compile implementation source files with precise header dependencies
$(BUILD_DIR)/implementation/%.o: implementation/%.cpp | $(IMPLEMENTATION_DIRS)
	$(CXX) $(CXXFLAGS) -MMD -MP -c $< -o $@

# Compile validation source files with precise header dependencies
$(BUILD_DIR)/validation/%.o: validation/%.cpp | $(VALIDATION_DIRS)
	$(CXX) $(CXXFLAGS) -MMD -MP -c $< -o $@

# Compile performance source files with precise header dependencies
$(BUILD_DIR)/performance/%.o: performance/%.cpp | $(PERFORMANCE_DIRS)
	$(CXX) $(CXXFLAGS) -MMD -MP -c $< -o $@

# Include all generated dependency files
-include $(BUILD_DIR)/implementation/*.d
-include $(BUILD_DIR)/validation/*/*.d
-include $(BUILD_DIR)/performance/*/*.d

# Validation target - run all validation tests (separate from build)
run-validation: build-validation
	@for test in $(VALIDATION_EXECS); do \
		echo "Running validation test: $$test"; \
		./$$test; \
	done

# Performance test target - run all performance tests (separate from build)
run-performance: build-performance
	@for test in $(PERF_EXECS); do \
		echo "Running performance test: $$test"; \
		./$$test; \
	done

# Run individual test directories
run-validation-%: build-validation
	@echo "Running validation tests in $* directory"
	@for test in $(BUILD_DIR)/validation/$*/*; do \
		if [ -x "$$test" ]; then \
			echo "Running validation test: $$test"; \
			./$$test; \
		fi \
	done

run-performance-%: build-performance
	@echo "Running performance tests in $* directory"
	@for test in $(BUILD_DIR)/performance/$*/*; do \
		if [ -x "$$test" ]; then \
			echo "Running performance test: $$test"; \
			./$$test; \
		fi \
	done

# Clang-format target
format:
	@if command -v clang-format >/dev/null 2>&1; then \
		echo "Running clang-format on all source files (excluding python_data_collection)..."; \
		clang-format -i -style=file $(FORMAT_SRCS); \
		echo "Formatting complete."; \
	else \
		echo "Error: clang-format not found. Please install clang-format."; \
		exit 1; \
	fi

# Clang-format check target (doesn't modify files)
format-check:
	@if command -v clang-format >/dev/null 2>&1; then \
		echo "Checking formatting of source files (excluding python_data_collection)..."; \
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
	rm -rf $(BUILD_DIR)

# Help target
help:
	@echo "Targets:"
	@echo "  all                : Build all test executables but don't run them (default)"
	@echo "  build-validation   : Build all validation test executables"
	@echo "  build-performance  : Build all performance test executables"
	@echo "  run-validation     : Build and run ALL validation tests"
	@echo "  run-performance    : Build and run ALL performance tests"
	@echo "  run-validation-X   : Run validation tests in subdirectory X (e.g., run-validation-mutex)"
	@echo "  run-performance-X  : Run performance tests in subdirectory X (e.g., run-performance-barrier)"
	@echo "  format             : Run clang-format on all source files (excluding python_data_collection)"
	@echo "  format-check       : Check formatting without modifying files (excluding python_data_collection)"
	@echo "  clean              : Remove all generated files"
	@echo "  help               : Show this help message"

# Phony targets
.PHONY: all clean build-all build-validation build-performance run-validation run-performance format format-check help
