# SPDX-License-Identifier: GPL-2.0

################################################################################
#                               COLOR DEFINITIONS                              #
################################################################################
END = \033[m
RED = \033[31m
GREEN = \033[32m
YELLOW = \033[33m
BLUE = \033[34m
LIGTH = \033[1m
DARK = \033[2m
ITALIC = \033[3m

SUCCESS = $(LIGTH)$(GREEN)[SUCCESS]$(END)
WARNING = $(LIGTH)$(YELLOW)[WARNING]$(END)
INFO = $(LIGTH)$(BLUE)[INFO]$(END)
ERROR = $(LIGTH)$(RED)[ERROR]$(END)

################################################################################
#                               BUILD VARIABLES                                #
################################################################################
RMV = rm -rf

# Directories
SRC_DIR = src
TEST_DIR = tests
OBJ_DIR = build
KERNEL_OBJ_DIR = .obj
KERNEL_DEP_DIR = .dep

# Cross-compiler configuration for kernel binary (i686-elf target)
CC = gcc
CXX = g++
LD = ld
ASM = nasm
ASM_FLAGS = -f elf32 -MD -MF
CFLAGS = -m32 -ffreestanding -O2 -Wall -Wextra -fno-builtin -nostdlib -I$(SRC_DIR)
LDFLAGS = -m elf_i386 -T $(LINKER) -nostdlib

# Test/coverage compiler configuration 
TEST_CFLAGS = -Wall -Wextra -O2 -fPIC -I. -I$(SRC_DIR)
TEST_CXXFLAGS = -Wall -Wextra -O2 -fPIC -I. -I$(SRC_DIR)

# Add coverage flags if COVERAGE is set
ifdef COVERAGE
	TEST_CFLAGS += -fprofile-arcs -ftest-coverage
	TEST_CXXFLAGS += -fprofile-arcs -ftest-coverage
	TEST_LDFLAGS += -fprofile-arcs -ftest-coverage
endif

# File definitions
LINKER = $(SRC_DIR)/boot/linker.ld
KERNEL_BIN = myos42.bin
ISO = myos42.iso
GRUBCFG = grub.cfg
TEST_RUNNER = test_runner

# Kernel source files
KERNEL_SOURCES_AS = $(SRC_DIR)/boot/entry.s
KERNEL_SOURCES_C = $(SRC_DIR)/kernel/main.c $(SRC_DIR)/kernel/display/display.c

# Test source files (kernel lib without main.c for testing)
KERNEL_LIB_SOURCES = $(SRC_DIR)/kernel/display/display.c
TEST_SOURCES = $(TEST_DIR)/unit/test_display.cpp

# Object files for kernel build
KERNEL_OBJECTS_AS = $(patsubst $(SRC_DIR)/%.s,$(KERNEL_OBJ_DIR)/%.o,$(KERNEL_SOURCES_AS))
KERNEL_OBJECTS_C = $(patsubst $(SRC_DIR)/%.c,$(KERNEL_OBJ_DIR)/%.o,$(KERNEL_SOURCES_C))
KERNEL_DEPENDENCIES_AS = $(patsubst $(SRC_DIR)/%.s,$(KERNEL_DEP_DIR)/%.d,$(KERNEL_SOURCES_AS))
KERNEL_DEPENDENCIES_C = $(patsubst $(SRC_DIR)/%.c,$(KERNEL_DEP_DIR)/%.d,$(KERNEL_SOURCES_C))

# Object files for test build
TEST_LIB_OBJECTS = $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(KERNEL_LIB_SOURCES))
TEST_OBJECTS = $(patsubst $(TEST_DIR)/unit/%.cpp,$(OBJ_DIR)/%.o,$(TEST_SOURCES))

# Google Test
GTEST_LIBS = -lgtest -lgtest_main

# Required tools
REQUIRED_TOOLS = qemu-system-x86_64 nasm grub-mkrescue $(CC)

# Subdirectories to create
KERNEL_SUBDIRS = boot kernel kernel/display
TEST_SUBDIRS = kernel/display

################################################################################
#                               PHONY TARGETS                                  #
################################################################################
.PHONY: all clean distclean re help check_requirements
.PHONY: kernel-bin iso run debug test test-verbose

################################################################################
#                               DEFAULT TARGET                                 #
################################################################################
all: check_requirements kernel-bin iso

################################################################################
#                               HELP TARGET                                    #
################################################################################
help:
	@printf "$(INFO) KFS Makefile - Available targets:\n\n"
	@printf "  $(BLUE)make all$(END)              $(INFO) Build kernel binary + ISO\n"
	@printf "  $(BLUE)make kernel-bin$(END)       $(INFO) Build only the kernel binary ($(KERNEL_BIN))\n"
	@printf "  $(BLUE)make iso$(END)              $(INFO) Create bootable ISO image ($(ISO))\n"
	@printf "  $(BLUE)make run$(END)              $(INFO) Run kernel in QEMU emulator\n"
	@printf "  $(BLUE)make debug$(END)            $(INFO) Run kernel in QEMU with GDB support\n"
	@printf "  $(BLUE)make test$(END)             $(INFO) Build and run unit tests\n"
	@printf "  $(BLUE)make test-verbose$(END)     $(INFO) Run tests with verbose output\n"
	@printf "  $(BLUE)make clean$(END)            $(INFO) Remove build artifacts\n"
	@printf "  $(BLUE)make distclean$(END)        $(INFO) Remove all generated files\n"
	@printf "  $(BLUE)make re$(END)               $(INFO) Rebuild everything from scratch\n"
	@printf "  $(BLUE)make help$(END)             $(INFO) Show this help message\n"
	@printf "  $(BLUE)make cross-help$(END)       $(INFO) Show cross-compiler setup targets\n\n"

################################################################################
#                               REQUIREMENTS CHECK                             #
################################################################################
check_requirements:
	@failed=false; \
	for tool in $(REQUIRED_TOOLS); do \
		if command -v $$tool >/dev/null 2>&1; then \
			printf "$(GREEN)$(LIGTH)[$$tool is installed.]$(END) Proceeding...\n"; \
		else \
			printf "$(RED)$(LIGTH)[$$tool is not installed.]$(END) Please install $$tool to proceed.\n"; \
			failed=true; \
		fi; \
	done; \
	if [ "$$failed" = true ]; then exit 1; fi
	@if [ ! -f $(LINKER) ]; then \
		printf "$(ERROR) Linker script $(LINKER) not found!\n"; \
		exit 1; \
	fi

################################################################################
#                               DIRECTORY CREATION                             #
################################################################################
$(KERNEL_OBJ_DIR):
	@mkdir -p $(KERNEL_OBJ_DIR)
	@$(foreach dir,$(KERNEL_SUBDIRS),mkdir -p $(KERNEL_OBJ_DIR)/$(dir);)
	@printf "$(INFO) Created kernel object directory structure\n"

$(KERNEL_DEP_DIR):
	@mkdir -p $(KERNEL_DEP_DIR)
	@$(foreach dir,$(KERNEL_SUBDIRS),mkdir -p $(KERNEL_DEP_DIR)/$(dir);)
	@printf "$(INFO) Created kernel dependency directory structure\n"

$(OBJ_DIR):
	@mkdir -p $(OBJ_DIR)
	@$(foreach dir,$(TEST_SUBDIRS),mkdir -p $(OBJ_DIR)/$(dir);)
	@printf "$(INFO) Created test object directory structure\n"

################################################################################
#                               KERNEL BUILD RULES                             #
################################################################################

# Assembly compilation rule
$(KERNEL_OBJ_DIR)/%.o: $(SRC_DIR)/%.s | $(KERNEL_OBJ_DIR) $(KERNEL_DEP_DIR)
	@printf "$(INFO) Assembling $< ...\n"
	@if $(ASM) $(ASM_FLAGS) $(KERNEL_DEP_DIR)/$*.d $< -o $@; then \
		printf "$(SUCCESS) Created: $@\n"; \
	else \
		printf "$(ERROR) Failed to assemble: $<\n"; \
		exit 1; \
	fi

# C compilation rule (cross-compiler)
$(KERNEL_OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(KERNEL_OBJ_DIR) $(KERNEL_DEP_DIR)
	@printf "$(INFO) Compiling $< ...\n"
	@if $(CC) $(CFLAGS) -MMD -MF $(KERNEL_DEP_DIR)/$*.d -c $< -o $@; then \
		printf "$(SUCCESS) Created: $@\n"; \
	else \
		printf "$(ERROR) Failed to compile: $<\n"; \
		exit 1; \
	fi

# Link kernel binary
kernel-bin: $(KERNEL_BIN)

$(KERNEL_BIN): $(KERNEL_OBJECTS_AS) $(KERNEL_OBJECTS_C) $(LINKER)
	@printf "$(INFO) Linking kernel binary: $(KERNEL_BIN) ...\n"
	@printf "$(INFO) Using linker script: $(LINKER)\n"
	@if $(LD) $(LDFLAGS) -o $@ $(KERNEL_OBJECTS_AS) $(KERNEL_OBJECTS_C); then \
		printf "$(SUCCESS) Created kernel: $(KERNEL_BIN)\n"; \
		printf "$(INFO) Kernel size: "; \
		wc -c < $(KERNEL_BIN) | awk '{printf "%d bytes (%.2f KB)\n", $$1, $$1/1024}'; \
	else \
		printf "$(ERROR) Failed to link kernel\n"; \
		exit 1; \
	fi

################################################################################
#                               GRUB & ISO                                     #
################################################################################
$(GRUBCFG):
	@printf "$(INFO) Creating GRUB configuration: $(GRUBCFG) ...\n"
	@echo 'set timeout=0' > $(GRUBCFG)
	@echo 'set default=0' >> $(GRUBCFG)
	@echo '' >> $(GRUBCFG)
	@echo 'menuentry "KFS - Kernel From Scratch" {' >> $(GRUBCFG)
	@echo '    multiboot /boot/$(KERNEL_BIN)' >> $(GRUBCFG)
	@echo '    boot' >> $(GRUBCFG)
	@echo '}' >> $(GRUBCFG)
	@printf "$(SUCCESS) Created: $(GRUBCFG)\n"

iso: $(ISO)

$(ISO): $(KERNEL_BIN) $(GRUBCFG)
	@printf "$(INFO) Creating bootable ISO: $(ISO) ...\n"
	@rm -rf iso 2>/dev/null || true
	@mkdir -p iso/boot/grub
	@cp $(KERNEL_BIN) iso/boot/
	@cp $(GRUBCFG) iso/boot/grub/
	@printf "$(INFO) Running grub-mkrescue...\n"
	@if grub-mkrescue -o $(ISO) --compress=xz iso 2>&1; then \
		printf "$(SUCCESS) ISO created successfully\n"; \
	else \
		printf "$(WARNING) grub-mkrescue failed, trying alternative...\n"; \
		if which xorriso >/dev/null 2>&1; then \
			xorriso -as mkisofs -R -b boot/grub/eltorito.img -no-emul-boot \
				-boot-load-size 4 -boot-info-table -o $(ISO) iso 2>/dev/null && \
			printf "$(SUCCESS) ISO created with xorriso\n"; \
		else \
			printf "$(ERROR) Failed to create ISO\n"; \
			exit 1; \
		fi; \
	fi
	@rm -rf iso
	@if [ -f $(ISO) ]; then \
		printf "$(SUCCESS) ISO file created: $(ISO)\n"; \
		printf "$(INFO) ISO size: "; \
		ls -lh "$(ISO)" | awk '{print $$5}'; \
	fi

################################################################################
#                               QEMU EXECUTION                                 #
################################################################################
run: $(ISO)
	@printf "$(INFO) Starting QEMU with kernel ...\n"
	@printf "$(INFO) Press Ctrl-A then X to exit QEMU\n"
	@qemu-system-x86_64 -cdrom $(ISO) -no-reboot -no-shutdown

debug: $(ISO)
	@printf "$(INFO) Starting QEMU with GDB support ...\n"
	@printf "$(INFO) Connect GDB with: gdb -ex 'target remote localhost:1234' -ex 'symbol-file $(KERNEL_BIN)'\n"
	@qemu-system-x86_64 -cdrom $(ISO) -s -S

################################################################################
#                               TEST BUILD RULES                               #
################################################################################

# Compile C sources for testing (using g++ for C/C++ compatibility)
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	@printf "$(INFO) Compiling $< for tests...\n"
	@$(CXX) $(TEST_CXXFLAGS) -c $< -o $@

# Compile C++ test files
$(OBJ_DIR)/%.o: $(TEST_DIR)/unit/%.cpp | $(OBJ_DIR)
	@printf "$(INFO) Compiling test $< ...\n"
	@$(CXX) $(TEST_CXXFLAGS) -c $< -o $@

# Link test runner
$(TEST_RUNNER): $(TEST_LIB_OBJECTS) $(TEST_OBJECTS)
	@printf "$(INFO) Linking test runner...\n"
	@$(CXX) $(TEST_CXXFLAGS) $(TEST_LDFLAGS) $^ -o $@ $(GTEST_LIBS)
	@printf "$(SUCCESS) Test runner built: $(TEST_RUNNER)\n"

# Build and run tests
test: $(TEST_RUNNER)
	@printf "$(INFO) Running tests...\n"
	@./$(TEST_RUNNER)

# Run tests with verbose output
test-verbose: $(TEST_RUNNER)
	@./$(TEST_RUNNER) --gtest_print_time=1 --gtest_verbose=1


################################################################################
#                               CLEANUP TARGETS                                #
################################################################################
clean:
	@printf "$(INFO) Cleaning build artifacts...\n"
	@$(RMV) $(OBJ_DIR) $(KERNEL_OBJ_DIR) $(KERNEL_DEP_DIR)
	@$(RMV) $(TEST_RUNNER) $(KERNEL_BIN)
	@find . -name "*.gcda" -delete 2>/dev/null || true
	@find . -name "*.gcno" -delete 2>/dev/null || true
	@printf "$(SUCCESS) Clean complete\n"

clean-coverage:
	@printf "$(INFO) Cleaning coverage reports...\n"
	@$(RMV) coverage_report build_coverage
	@find . -name "*.gcda" -delete 2>/dev/null || true
	@find . -name "*.gcno" -delete 2>/dev/null || true
	@printf "$(SUCCESS) Coverage clean complete\n"

distclean: clean clean-coverage
	@printf "$(INFO) Cleaning all generated files...\n"
	@$(RMV) $(ISO) $(GRUBCFG)
	@printf "$(SUCCESS) Distclean complete\n"

################################################################################
#                               REBUILD TARGET                                 #
################################################################################
re: distclean all

################################################################################
#                               DEPENDENCY INCLUSION                           #
################################################################################
-include $(KERNEL_DEPENDENCIES_AS)
-include $(KERNEL_DEPENDENCIES_C)

.SILENT: