# KFS Makefile
# Compiles kernel source and tests

# Compiler settings
CC = gcc
CXX = g++
CFLAGS = -Wall -Wextra -O2 -fPIC
CXXFLAGS = -Wall -Wextra -O2 -fPIC
CPPFLAGS = -I. -Isrc

# Add coverage flags if COVERAGE is set
ifdef COVERAGE
	CFLAGS += -fprofile-arcs -ftest-coverage
	CXXFLAGS += -fprofile-arcs -ftest-coverage
	LDFLAGS += -fprofile-arcs -ftest-coverage
endif

# Directories
SRC_DIR = src
TEST_DIR = tests
OBJ_DIR = build
BIN_DIR = .

# Source files
SOURCES = $(SRC_DIR)/math/math.c $(SRC_DIR)/main.c
# Test source files (kernel sources without main.c for testing)
KERNEL_LIB_SOURCES = $(SRC_DIR)/math/math.c
TEST_SOURCES = $(TEST_DIR)/unit/test_math.cpp
KERNEL_OBJECTS = $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SOURCES))
KERNEL_LIB_OBJECTS = $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(KERNEL_LIB_SOURCES))
TEST_OBJECTS = $(patsubst $(TEST_DIR)/unit/%.cpp,$(OBJ_DIR)/%.o,$(TEST_SOURCES))

# Output files
TEST_RUNNER = test_runner
KERNEL_BIN = kernel

# Google Test
GTEST_DIR = /usr/src/gtest
GTEST_LIBS = -lgtest -lgtest_main

# All target - build kernel binary
all: $(BIN_DIR)/$(KERNEL_BIN)

# Create build directory
$(OBJ_DIR):
	@mkdir -p $(OBJ_DIR)
	@mkdir -p $(OBJ_DIR)/math

# Compile C source files (using g++ for C/C++ test compatibility)
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c $< -o $@

# Build kernel binary
$(BIN_DIR)/$(KERNEL_BIN): $(KERNEL_OBJECTS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $^ -o $@
	@echo "Kernel binary built: $@"

# Compile C++ test files
$(OBJ_DIR)/%.o: $(TEST_DIR)/unit/%.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c $< -o $@

# Link test runner
$(BIN_DIR)/$(TEST_RUNNER): $(KERNEL_LIB_OBJECTS) $(TEST_OBJECTS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $^ -o $@ $(GTEST_LIBS)

# Clean build artifacts
clean:
	@rm -rf $(OBJ_DIR)
	@rm -f $(BIN_DIR)/$(TEST_RUNNER)
	@rm -f $(BIN_DIR)/$(KERNEL_BIN)
	@rm -f *.gcda *.gcno
	@echo "Clean complete"

# Clean coverage reports
clean-coverage:
	@rm -rf coverage_report
	@rm -rf build_coverage
	@find . -name "*.gcda" -delete
	@find . -name "*.gcno" -delete
	@echo "Coverage clean complete"

# Very clean (everything)
distclean: clean clean-coverage

# Build and run tests
test: $(BIN_DIR)/$(TEST_RUNNER)
	./$(TEST_RUNNER)

# Previous test target renamed
test-runner: $(BIN_DIR)/$(TEST_RUNNER)

# Run tests with verbose output
test-verbose: $(BIN_DIR)/$(TEST_RUNNER)
	./$(TEST_RUNNER) --gtest_print_time=1 --gtest_verbose=1

# Run specific test
test-filter: $(BIN_DIR)/$(TEST_RUNNER)
	./$(TEST_RUNNER) --gtest_filter="$(FILTER)"

# Help
help:
	@echo "KFS Makefile targets:"
	@echo "  make              - Build kernel binary"
	@echo "  make test         - Build and run unit tests"
	@echo "  make test-verbose - Run tests with verbose output"
	@echo "  make test-filter FILTER=<pattern> - Run specific tests"
	@echo "  make clean        - Remove build artifacts"
	@echo "  make distclean    - Remove everything including coverage"
	@echo "  make help         - Show this help message"

.PHONY: all clean clean-coverage distclean test test-verbose test-filter help
