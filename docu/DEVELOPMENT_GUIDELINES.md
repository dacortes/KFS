# KFS - Software Development Guidelines

This document establishes the standards and conventions for the KFS (Kernel From Scratch) project. All developers must adhere to these guidelines to maintain code consistency, quality, and readability.

## 1. Architecture

### Target Architecture
- **Mandatory**: x86 (Intel 386) architecture
- All assembly code must be compatible with 32-bit x86 processors
- When working with architecture-specific code, clearly document target platform

### Architecture Considerations
- Document any assembly-specific optimizations or dependencies
- Use preprocessor directives (`#ifdef`, `#ifdef __i386__`) for architecture-specific code blocks
- Keep architecture-independent code separate from platform-specific implementations

---

## 2. Code Style

### General Principles
- Consistency is paramount - follow existing patterns in the codebase
- Use the Linux kernel's `checkpatch.pl` script for automatic style validation
- All code must compile without warnings (`-Wall -Wextra` flags)
- All code must pass the linter before submission

### Running checkpatch.pl
```bash
./scripts/checkpatch.pl --no-tree --file <filename.c>
./scripts/checkpatch.pl --no-tree --file <filename.asm>
```

The `--no-tree` flag allows checkpatch.pl to work with standalone projects outside a kernel tree.

### C Code Style Guidelines

#### Indentation
- Use **tabs** (not spaces) for indentation
- One tab = 8 characters

#### Line Length
- Maximum line length: **80 characters**
- Break long lines appropriately

#### Braces
```c
/* Opening brace on same line, closing brace on new line */
if (condition) {
    statement;
}

/* Function definitions - opening brace on new line */
void function_name(void)
{
    /* code */
}
```

#### Variable and Function Declarations
```c
/* Pointer asterisk with variable name, not type */
char *ptr;
int *count;

/* Multiple declarations on separate lines */
int x;
int y;
int z;
```

#### Spacing
- One space after keywords (`if`, `while`, `for`, `switch`, `return`)
- No space between function name and parentheses: `function()`
- Spaces around binary operators: `a + b`, `x = 5`
- No spaces inside parentheses or brackets

#### Comments
- **Minimize inline comments** - code should be self-documenting through clear naming
- Use comments **only when logic is non-obvious or requires explanation**
- Avoid stating the obvious: `i++; /* increment i */` ❌
- Prefer clear variable names and function extraction over comments

```c
/* BAD: Obvious comment */
x = x + 1; /* Add one to x */
if (count > 0) { /* Check if count is positive */
    process();
}

/* GOOD: Self-documenting code, minimal comments */
count++;
if (count > 0) {
    process();
}

/* GOOD: Comment only for non-obvious logic */
/* Calculate CRC using polynomial 0x1021 */
crc = (crc << 1) ^ ((data & 0x80) ? 0x1021 : 0);
```

**Inside Function Rule**: 
- Comments inside functions should be **extremely rare**
- If you need a comment to explain what code does, refactor into a separate function with a clear name
- Comments are acceptable for complex algorithms or non-standard techniques

### Assembly Code Style Guidelines

#### Formatting
- Use **tabs** for indentation (consistent with C code)
- Maximum line length: **80 characters**
- One instruction per line

#### Comments
```asm
; Register usage comment at function entry
; eax: return value
; ebx: loop counter
; ecx: temporary variable

section .text
    global asm_function
asm_function:
    ; Initialize
    xor eax, eax
    mov ebx, 0
    ret
```

#### Labels and Instructions
- Labels start at column 0 (no indentation)
- Instructions indented with one tab
- Mnemonics in lowercase

```asm
.start:
    mov eax, 0x00000000
    jmp .loop

.loop:
    inc eax
    cmp eax, 10
    jne .loop
    ret
```

---

## 3. Naming Conventions

### Language
- **All function names**: English
- **All variable names**: English
- **All comments**: English
- **All documentation**: English

### C Naming Conventions

#### Functions
- Use **snake_case** for function names
- Descriptive names that indicate purpose
- Prefix with module/subsystem name if appropriate

```c
void memory_init(void);
int process_create(const char *name);
void timer_interrupt_handler(void);
```

#### Variables
- Use **snake_case** for variable names
- Descriptive names that indicate purpose and content
- Use meaningful abbreviations only

```c
int process_count;
unsigned int memory_size;
struct memory_block *current_block;
```

#### Constants and Macros
- Use **UPPERCASE_SNAKE_CASE**
- Descriptive names

```c
#define MAX_PROCESSES 256
#define KERNEL_STACK_SIZE 0x1000
#define PAGE_SIZE 4096
```

#### Structs and Typedefs
- Struct names: **snake_case**
- Typedef names: **snake_case** with `_t` suffix

```c
struct process_control_block {
    int pid;
    char name[64];
    int priority;
};

typedef struct process_control_block process_control_block_t;
```

#### Pointers to Functions
- Use descriptive names indicating the function's purpose

```c
typedef int (*process_handler_t)(int arg);
typedef void (*interrupt_handler_t)(int irq);
```

### Assembly Naming Conventions

#### Labels
- Use **snake_case** preceded by a dot (`.label_name`)
- Descriptive names indicating function or section purpose
- Local labels prefixed with function name

```asm
asm_function:
    ; code
    .check_condition:
        cmp eax, 0
        je .handle_zero
    
    .handle_nonzero:
        ; code
        ret
    
    .handle_zero:
        ; code
        ret
```

#### Registers
- Document register usage clearly in function prologues
- Use consistent register allocation throughout a function

```asm
; Function: calculate_sum
; Input: esi = array pointer, eax = array length
; Output: eax = sum
; Clobbered: ecx, edx
```

---

## 4. Object-Oriented Design Pattern

### Core Design Principle
Each functionality must be encapsulated as an **Object** using structs with function pointers and associated state variables.

### Structure Pattern
```c
/* Object definition */
typedef struct {
    /* State variables */
    int state;
    unsigned int count;
    void *data;
    
    /* Function pointers (methods) */
    int (*init)(struct object_t *self);
    int (*process)(struct object_t *self, int arg);
    int (*cleanup)(struct object_t *self);
} object_t;

/* Constructor pattern */
object_t* object_create(void)
{
    object_t *obj = memory_allocate(sizeof(object_t));
    obj->state = 0;
    obj->count = 0;
    obj->data = NULL;
    
    /* Assign method pointers */
    obj->init = object_init;
    obj->process = object_process;
    obj->cleanup = object_cleanup;
    
    return obj;
}

/* Method implementations */
static int object_init(object_t *self)
{
    self->state = 1;
    return 0;
}

static int object_process(object_t *self, int arg)
{
    /* Process argument */
    return 0;
}

/* Usage */
int main(void)
{
    object_t *obj = object_create();
    obj->init(obj);
    obj->process(obj, 42);
    obj->cleanup(obj);
    memory_free(obj);
}
```

### Benefits
- Encapsulation of state and behavior
- Clear separation of concerns
- Easier to maintain and extend
- Supports multiple implementations of same interface

---

## 5. Function Arguments

### Maximum Arguments Guideline
- **Maximum 3 arguments per function**
- For more parameters, use structs as arguments

### Guidelines

#### Acceptable (3 or fewer arguments)
```c
int create_process(const char *name, int priority, int parent_pid);
void set_memory_region(unsigned int start, unsigned int size, int flags);
```

#### Too many arguments - Use struct instead
```c
/* BAD: Too many arguments */
int create_process(const char *name, int priority, int parent_pid, 
                   int memory_pages, int stack_size, int heap_size,
                   int file_descriptor);

/* GOOD: Use struct for configuration */
typedef struct {
    const char *name;
    int priority;
    int parent_pid;
    int memory_pages;
    int stack_size;
    int heap_size;
    int file_descriptor;
} process_config_t;

int create_process(const process_config_t *config);
```

### Return Values
- Return structs when multiple values need to be returned
- Use output parameters only when necessary

```c
/* Good: Return struct for multiple values */
typedef struct {
    int result;
    int error_code;
    char *message;
} operation_result_t;

operation_result_t perform_operation(int arg);

/* Good: Single return value with pointer output parameter */
int get_process_info(int pid, process_info_t *info);
```

---

## 6. Documentation

### Docstring Format

All functions must have a docstring following this format:

```c
/**
 * Brief description of what the function does.
 * 
 * Longer description explaining the function's purpose, behavior,
 * and any important details. Include algorithm complexity if relevant.
 * 
 * @param param1_name Description of param1
 * @param param2_name Description of param2
 * @return Description of return value (0 on success, error code on failure)
 * 
 * @note Any important notes about function behavior or limitations
 * @see Related functions or documentation
 */
int function_name(int param1_name, const char *param2_name);
```

### Docstring Examples

#### Example 1: Simple function
```c
/**
 * Initializes the memory management subsystem.
 * 
 * Sets up the heap, initializes memory pools, and prepares
 * the allocator for accepting allocation requests.
 * 
 * @return 0 on success, -1 if initialization fails
 * 
 * @note Must be called before any memory allocation
 */
int memory_init(void);
```

#### Example 2: Complex function
```c
/**
 * Creates a new process with the specified configuration.
 * 
 * Allocates a new process control block, sets up virtual memory
 * space, initializes the stack and heap, and adds the process
 * to the scheduler's ready queue.
 * 
 * @param config Pointer to process configuration structure containing
 *               name, priority, memory requirements, etc.
 * @return Process ID (PID) of newly created process on success,
 *         -1 if process creation fails
 * 
 * @note Maximum number of processes is limited by MAX_PROCESSES
 * @see process_destroy(), process_config_t
 */
int process_create(const process_config_t *config);
```

### Struct Documentation
```c
/**
 * Process Control Block - represents a single process
 * 
 * Stores all information necessary to manage a process's
 * execution, memory, and resources.
 */
typedef struct {
    int pid;              /**< Unique process identifier */
    char name[64];        /**< Process name/command */
    int priority;         /**< Priority level (0-31, 31=highest) */
    unsigned int sp;      /**< Stack pointer */
    unsigned int ip;      /**< Instruction pointer */
    int state;            /**< Process state (READY, RUNNING, BLOCKED) */
} process_control_block_t;
```

### Documentation Placement

**Header Files (.h)**:
- Include all docstrings for public functions, structs, macros, and types
- This is the primary documentation location for the public API
- Docstrings explain the contract and behavior of public interfaces

**Implementation Files (.c)**:
- Include file header with `@file` and `@brief` tags only
- Do NOT repeat docstrings from header file
- Include comments explaining complex implementation details or algorithms
- Use inline comments to clarify non-obvious logic

Example structure:
```c
/* Implementation file - math.c */
/**
 * @file math.c
 * @brief Basic math utility functions implementation
 */

#include "math.h"

/* Function implementation - docstring is in math.h */
int math_add(int a, int b)
{
    return a + b;
}
```

### Assembly Function Documentation
```asm
;**
; * Brief description of function
; * 
; * Detailed description of what the function does.
; * 
; * @input eax - Description of input in eax
; * @input esi - Description of input in esi
; * @output eax - Description of output in eax
; * @clobber ecx, edx - Registers that will be modified
; * 
; * @note Any important notes
;**/
```

### Include File Standards

**System and Library Includes**:
- Use angle brackets: `<stdio.h>`, `<gtest/gtest.h>`
- Place at top of file before local includes

**Local Project Includes**:
- Use angle brackets with proper `-I` compiler flag path: `<math.h>`
- Do NOT use relative paths: `#include "../src/math.h"` ❌
- Makefile must specify include directories with `-I` flag: `CPPFLAGS = -I. -Isrc`

Example:
```c
/* Correct: Using -I flag in Makefile and <> brackets */
#include <stdio.h>      /* System library */
#include <gtest/gtest.h> /* Third-party library */
#include <math.h>       /* Local project header */

/* Incorrect: Relative paths */
#include "../src/math.h" /* ❌ Avoid this */
#include "./math.h"      /* ❌ Avoid this */
```

### Additional Documentation

#### File Header and License Identifier

Every source file must start with an SPDX license identifier:

```c
// SPDX-License-Identifier: GPL-2.0

/**
 * @file filename.c
 * @brief Brief description of module
 * 
 * Detailed description of what this module does, its role in
 * the kernel, and any important implementation details.
 */
```

The SPDX-License-Identifier must be on the first line, followed by the documentation block.

---

## 7. Testing

### Testing Framework
- **Framework**: GoogleTest (gtest)
- All new functionality must have corresponding unit tests
- Target: Minimum 80% code coverage for critical modules

### Test File Organization
```
tests/
├── unit/
│   ├── test_memory.cpp
│   ├── test_process.cpp
│   └── test_interrupt.cpp
├── integration/
│   ├── test_process_memory.cpp
│   └── test_scheduler.cpp
└── fixtures/
    └── test_helpers.h
```

**Build Output**: Compiled test executable at `./test_runner`

### Writing Tests

#### Basic Test Structure
```cpp
#include <gtest/gtest.h>
#include "memory.h"

class MemoryTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Initialize before each test
        memory_init();
    }
    
    void TearDown() override {
        // Cleanup after each test
        memory_cleanup();
    }
};

TEST_F(MemoryTest, AllocateReturnsValidPointer) {
    void *ptr = memory_allocate(1024);
    EXPECT_NE(nullptr, ptr);
    memory_free(ptr);
}

TEST_F(MemoryTest, AllocateZeroReturnsNull) {
    void *ptr = memory_allocate(0);
    EXPECT_EQ(nullptr, ptr);
}

TEST_F(MemoryTest, DoubleFreeDetected) {
    void *ptr = memory_allocate(1024);
    memory_free(ptr);
    EXPECT_DEATH(memory_free(ptr), "double free detected");
}
```

#### Test Naming Convention
- Test names: `Test<ComponentName><SpecificScenario>`
- Use descriptive names that explain what is being tested

```cpp
TEST_F(ProcessTest, CreateProcessWithValidConfig);
TEST_F(ProcessTest, CreateProcessFailsWithNullConfig);
TEST_F(ProcessTest, DestroyProcessFreesAllResources);
```

### Running Tests
```bash
# Build project and tests with Makefile
make clean
make

# Run all GoogleTest tests
./test_runner

# Run specific test suite
./test_runner --gtest_filter="MemoryTest.*"

# Run with verbose output
./test_runner --gtest_print_time=1 --gtest_verbose=1

# Generate coverage report
./coverage.sh
```

---

## 8. Continuous Integration

### GitHub Actions Workflow
Automated testing on every push and pull request:

```yaml
name: KFS CI

on: [push, pull_request]

jobs:
  build-and-test:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v3
      
      - name: Install dependencies
        run: |
          sudo apt-get update
          sudo apt-get install -y gcc g++ nasm lcov
      
      - name: Run checkpatch
        run: |
          if [ -f ./scripts/checkpatch.pl ]; then
            ./scripts/checkpatch.pl --file src/*.c
          fi
      
      - name: Build
        run: |
          make clean
          make -j$(nproc)
      
      - name: Run tests
        run: |
          if [ -f ./test ]; then
            ./test
          fi
      
      - name: Generate coverage
        run: |
          if [ -f ./coverage.sh ]; then
            chmod +x ./coverage.sh
            ./coverage.sh
          fi
      
      - name: Upload coverage to artifacts
        if: always()
        uses: actions/upload-artifact@v3
        with:
          name: coverage-report
          path: coverage_report/
```

### Checks Before Commit
- Code passes `checkpatch.pl`
- All tests pass
- Code coverage maintained or improved
- Documentation is complete and accurate

---

## 9. Version Control Practices

### Pull Request Requirements
- All tests must pass
- Code must pass checkpatch validation
- Docstrings for all public functions
- Update relevant documentation
- Minimum one review before merge

---

## 10. Development Workflow

### Step-by-step process for implementing a feature:

1. **Create branch** from `main`
   ```bash
   git checkout -b feature/my-feature
   ```

2. **Implement feature** following all guidelines in this document

3. **Write tests** before or during implementation (TDD recommended)

4. **Validate code style**
   ```bash
   ./scripts/checkpatch.pl --file src/my_file.c
   ```

5. **Run tests and coverage**
   ```bash
   ./build/tests/unit_tests
   ./coverage.sh
   ```

6. **Add documentation**
   - Docstrings for all functions
   - Update README if needed
   - Add to API documentation

7. **Commit with descriptive message**
   ```bash
   git commit -m "[COMPONENT] Feature description"
   ```

8. **Push and create pull request**
   ```bash
   git push origin feature/my-feature
   ```

9. **Code review and merge**
   - Address review comments
   - Ensure all CI checks pass
   - Merge to main branch

---

## 11. Quick Reference Checklist

Before submitting code, verify:

- [ ] Code compiles without warnings (`-Wall -Wextra`)
- [ ] Code passes `checkpatch.pl --no-tree` without warnings
- [ ] All function parameters ≤ 3 (or use struct)
- [ ] All public functions have docstrings
- [ ] Unit tests written and passing
- [ ] No compiler warnings
- [ ] Variable/function names in English
- [ ] Comments and documentation in English
- [ ] Code follows x86 architecture guidelines
- [ ] Related functions encapsulated in structs (OOP pattern)
- [ ] Commit message follows format: `[COMPONENT] Description`

---

## 12. Resources and References

### Linux checkpatch.pl
- Location: `./scripts/checkpatch.pl`
- Usage: `./scripts/checkpatch.pl --help`
- Reference: Linux kernel coding style guide

### GoogleTest Documentation
- https://google.github.io/googletest/
- Framework for unit testing C/C++

### x86 Architecture
- Intel 386 instruction set reference
- i386 calling conventions and ABI

---

**Last Updated**: February 3, 2026
**Status**: Active
