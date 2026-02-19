# KFS Coding Agent Instructions

## Repository Overview
**KFS (Kernel From Scratch)** is a small educational operating system kernel project targeting x86 (Intel 386) 32-bit architecture. The project emphasizes Linux kernel coding standards, test-driven development, and clean architecture using object-oriented patterns in C.

**Size**: ~10 source files | **Languages**: C, C++ (tests only), x86 Assembly (NASM) | **Test Framework**: GoogleTest

## Critical: Read This First
**ALWAYS** consult `DEVELOPMENT_GUIDELINES.md` in the repository root - it contains mandatory coding standards, architectural patterns, and conventions. This file supplements those guidelines with practical build/test workflows.

## Build & Validation Workflow

### Required Tools (Pre-installed on Ubuntu CI)
- gcc/g++ 11.4.0+ (compiler)
- nasm 2.15+ (assembler)
- lcov 1.14+ (coverage)
- cppcheck (static analysis)
- libgtest-dev (unit tests)

### Build Commands (In Order)
```bash
# 1. Clean (always start here after making changes)
make clean

# 2. Build kernel binary (creates build/ directory automatically)
make              # Builds kernel binary at ./kernel

# 3. Build and run tests
make test         # Builds ./test_runner and executes it
# Alternative: make test-runner (builds only, doesn't run)

# 4. Run specific tests (optional)
make test-filter FILTER="DisplayTest.*"

# 5. Verify code style with checkpatch.pl
./scripts/style_check.sh

# 6. Run static analysis
./scripts/static_analysis.sh

# 7. Generate coverage report (target: 80%+)
./scripts/coverage.sh

# 8. Run ALL checks (recommended before commits)
./scripts/run_all_checks.sh
```

**IMPORTANT**: The Makefile automatically creates the `build/` directory structure - do not create it manually.

### Build Order Rules
✅ **DO**: Always run `make clean` before rebuilding after changes
✅ **DO**: Run `./scripts/run_all_checks.sh` before committing
✅ **DO**: Verify tests pass with `make test` after code changes
❌ **DON'T**: Run coverage.sh without cleaning first - it runs `make clean` internally
❌ **DON'T**: Manually create build directories - Makefile handles this

### Pre-Commit Validation (CI Pipeline Replication)
The GitHub Actions CI runs these steps in sequence:
```bash
./scripts/style_check.sh       # Must pass (0 errors, 0 warnings)
./scripts/static_analysis.sh   # Must pass (cppcheck)
./scripts/build.sh             # Must build successfully
./scripts/coverage.sh          # Must generate report (80%+ target)
```

**Tip**: Run `./scripts/run_all_checks.sh` to replicate CI locally (~60 seconds).

## Project Architecture

### Directory Structure
```
KFS/
├── src/                    # All kernel source code
│   ├── main.c             # Kernel entry point
│   └── display/           # Display subsystem (OOP pattern example)
│       ├── display.h      # Public API
│       └── display.c      # Implementation
├── tests/
│   ├── unit/              # GoogleTest unit tests (C++)
│   │   └── test_display.cpp
│   ├── integration/       # (future integration tests)
│   └── fixtures/          # Test helpers
├── scripts/               # Build and validation scripts
│   ├── build.sh           # Main build script
│   ├── style_check.sh     # checkpatch.pl runner
│   ├── static_analysis.sh # cppcheck runner
│   ├── coverage.sh        # lcov coverage generator
│   └── run_all_checks.sh  # Complete validation suite
├── build/                 # Auto-generated object files (gitignored)
├── coverage_report/       # Auto-generated coverage (gitignored)
├── Makefile               # Primary build system
└── DEVELOPMENT_GUIDELINES.md  # **MANDATORY READING**
```

### Key Configuration Files
- **Makefile**: Build system with coverage support (`COVERAGE=1 make`)
- **DEVELOPMENT_GUIDELINES.md**: Complete coding standards (function args, OOP patterns, docstrings, etc.)
- **.github/workflows/ci.yml**: GitHub Actions CI/CD pipeline
- **.gitignore**: Excludes `build/`, `*.o`, `*.gcda`, `*.gcno`, coverage reports

### Architectural Patterns (CRITICAL)
1. **Object-Oriented Design in C**: Use structs with function pointers (see `display_t` in `src/display/`)
2. **Maximum 3 Function Arguments**: Use structs for more parameters
3. **SPDX License Identifier**: First line of every file: `// SPDX-License-Identifier: GPL-2.0`
4. **Docstrings Required**: All public functions need docstrings (Doxygen style with `@param`, `@return`)
5. **Include Paths**: Use `<module/header.h>` (not relative paths) - Makefile sets `-Isrc`
6. **Header Guards**: Use `#pragma once` (not traditional `#ifndef` guards)

## Code Style Enforcement

### Mandatory Style Rules (enforced by checkpatch.pl)
- **Indentation**: Tabs only (8 chars), no spaces
- **Line length**: 80 characters max
- **Braces**: K&R style (`if (x) {` on same line, functions have `{` on new line)
- **Naming**: `snake_case` for functions/variables, `UPPER_CASE` for macros
- **Comments**: Minimize inside functions - code should be self-documenting
- **Pointers**: Asterisk with variable (`char *ptr`, not `char* ptr`)

### Common Checkpatch Warnings
```c
/* ❌ WRONG */
if(condition){  // Missing space after 'if', before '{'
    return 0;
}
char* ptr;      // Wrong pointer style

/* ✅ CORRECT */
if (condition) {
    return 0;
}
char *ptr;
```

**Note**: checkpatch.pl warnings about missing `spelling.txt` and `const_structs.checkpatch` are expected and safe to ignore.

## Testing Requirements

### Writing Tests
- **Framework**: GoogleTest (C++)
- **Location**: `tests/unit/test_<module>.cpp`
- **Pattern**: Use `TEST_F` with fixture classes for state setup
- **Coverage Target**: 80% line coverage minimum (enforced by CI)

### Test Compilation Details
- C source files compiled with `g++` (for C/C++ compatibility)
- Tests link against kernel sources (excluding `main.c`)
- GoogleTest flags: `-lgtest -lgtest_main`

### Running Tests
```bash
# Quick test run
make test

# Verbose output
make test-verbose

# Specific test suite
make test-filter FILTER="DisplayTest.ClearFillsWithSpaces"
```

## Common Pitfalls & Solutions

### Build Issues
**Problem**: `undefined reference to display_init`
- **Cause**: Missing source file in Makefile `SOURCES` or `KERNEL_LIB_SOURCES`
- **Fix**: Add new .c files to appropriate variable in Makefile

**Problem**: Coverage shows 0% or empty report
- **Cause**: Not cleaning before coverage build
- **Fix**: `make clean` then `./scripts/coverage.sh` (coverage.sh does this automatically)

**Problem**: Build directory not found
- **Cause**: Old/manual build configs
- **Fix**: Remove manual `mkdir` commands - Makefile creates `$(OBJ_DIR)` automatically

### Code Style Issues
**Problem**: checkpatch.pl reports "use tabs not spaces"
- **Fix**: Configure editor for tabs (8-char width) - check DEVELOPMENT_GUIDELINES.md

**Problem**: Line too long
- **Fix**: Break at 80 chars, indent continuation with 2 tabs

### Test Issues
**Problem**: GoogleTest not found during linking
- **Fix**: Install with `sudo apt-get install libgtest-dev`

## File Templates

### New C Source File
```c
// SPDX-License-Identifier: GPL-2.0

/**
 * @file module_name.c
 * @brief Brief description of module purpose
 */

#include <module/module_name.h>

/* Implementation here */
```

### New Header File
```c
// SPDX-License-Identifier: GPL-2.0

/**
 * @file module_name.h
 * @brief Brief description of public API
 */

#pragma once

/* Declarations here */
```

### New Unit Test
```cpp
// SPDX-License-Identifier: GPL-2.0

#include <gtest/gtest.h>
#include <module/module_name.h>

class ModuleTest : public ::testing::Test {
protected:
    void SetUp() override {
        /* Setup before each test */
    }
};

TEST_F(ModuleTest, DescriptiveTestName) {
    /* Test implementation */
    EXPECT_EQ(expected, actual);
}
```

## Quick Reference: Make Targets
```bash
make              # Build kernel binary (./kernel)
make test         # Build and run all tests
make test-verbose # Run tests with timing info
make test-filter FILTER="pattern"  # Run specific tests
make clean        # Remove build artifacts
make distclean    # Remove build + coverage
make help         # Show all targets
```

## Agent Instructions
**TRUST THESE INSTRUCTIONS**: Only search/explore if information here is incomplete or contradicts actual file contents. For any coding standards questions, refer to `DEVELOPMENT_GUIDELINES.md` first. When making changes:

1. Read relevant source files completely before editing
2. Follow the exact code style (tabs, 80 chars, snake_case)
3. Add SPDX identifier and docstrings to new files
4. Run `make clean && make test` after changes
5. Use `./scripts/run_all_checks.sh` before claiming completion
6. Check CI workflow (`.github/workflows/ci.yml`) if CI fails

**Success Criteria**: Code compiles without warnings, tests pass (100% ideally), checkpatch.pl reports 0 errors/warnings, static analysis passes, coverage remains ≥80%.
