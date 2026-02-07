################################################################################
#                               COLOR DEFINITIONS                              #
# Used for colored terminal output                                             #
################################################################################

END = \033[m
RED = \033[31m
GREEN = \033[32m
YELLOW = \033[33m
BLUE = \033[34m
LIGTH = \033[1m
DARK = \033[2m
ITALIC = \033[3m

################################################################################
#                               MESSAGE LABELS                                 #
# Predefined colored messages for consistent output                            #
################################################################################

SUCCESS = $(LIGTH)$(GREEN)[SUCCESS]$(END)
WARNING = $(LIGTH)$(YELLOW)[WARNING]$(END)
INFO = $(LIGTH)$(BLUE)[INFO]$(END)
ERROR = $(LIGTH)$(RED)[ERROR]$(END)

################################################################################
#                               BUILD VARIABLES                                #
# Compiler, flags, and file definitions                                        #
################################################################################

RMV      = rm -rf
KERNEL   = myos42.bin                   # Final kernel binary name
ISO      = myos42.iso                   # Bootable ISO image name
GRUBCFG  = grub.cfg                     # GRUB configuration file
LINKER   = linker.ld                    #Linker script file

# Assembler configuration (NASM for x86 assembly)
ASM      = nasm -f elf32 #-w+all -w+error -w+orphan-labels -g
FLAGS_ASM = -MD -MF                # Generate dependency files

# Cross-compiler configuration for i686-elf target
CC       = i686-elf-gcc            # Cross-compiler for bare-metal code
LD       = i686-elf-ld             # Cross-linker for ELF binaries
CFLAGS   = -m32 -ffreestanding -O2 -Wall -Wextra -fno-builtin -nostdlib
LDFLAGS  = -m elf_i386 -T $(LINKER) -nostdlib

# Source file definitions
SOURCES_AS := boot/entry.s         # Assembly entry point (bootloader)
SOURCES_C  := kernel/main.c        # Main C kernel source

# Build directories
DIRECTORY_OBJ = .obj
DIRECTORY_DEP = .dep
SUB_DIRECTORIES := boot kernel

# Object and dependency file paths (pattern substitution)
OBJECTS = $(addprefix $(DIRECTORY_OBJ)/, $(SOURCES_AS:.s=.o))
DEPENDENCIES = $(addprefix $(DIRECTORY_DEP)/, $(SOURCES_AS:.s=.d))
OBJECTS_C = $(addprefix $(DIRECTORY_OBJ)/, $(SOURCES_C:.c=.o))
DEPENDENCIES_C = $(addprefix $(DIRECTORY_DEP)/, $(SOURCES_C:.c=.d))
DIRS_TO_CREATE = $(DIRECTORY_OBJ) $(DIRECTORY_DEP)

# Required tools for building and testing
REQUIRED_TOOLS := qemu-system-x86_64 nasm gcc grub-mkrescue $(CC)

################################################################################
#                               PHONY TARGETS                                  #
# Targets that don't represent actual files                                    #
################################################################################
.PHONY: all dir kernel iso run debug check_requirements clean distclean re help

################################################################################
#                               DEFAULT TARGET                                 #
# Builds everything: directories, objects, kernel binary, and ISO              #
################################################################################
#all: check_requirements dir $(OBJECTS)
all: check_requirements dir kernel iso

################################################################################
#                               HELP TARGET                                    #
# Displays available make targets with descriptions                            #
################################################################################
help:
	@printf "$(INFO) Available targets:\n\n"
	@printf "  $(BLUE)make all$(END)              $(INFO) Build everything (kernel + ISO)\n"
	@printf "  $(BLUE)make dir$(END)              $(INFO) Create required build directories\n"
	@printf "  $(BLUE)make kernel$(END)           $(INFO) Build only the kernel binary ($(KERNEL))\n"
	@printf "  $(BLUE)make iso$(END)              $(INFO) Create bootable ISO image ($(ISO))\n"
	@printf "  $(BLUE)make run$(END)              $(INFO) Run kernel in QEMU emulator\n"
	@printf "  $(BLUE)make debug$(END)            $(INFO) Run kernel in QEMU with GDB support\n"
	@printf "  $(BLUE)make clean$(END)            $(INFO) Remove object and dependency files\n"
	@printf "  $(BLUE)make distclean$(END)        $(INFO) Remove all generated files\n"
	@printf "  $(BLUE)make re$(END)               $(INFO) Rebuild everything from scratch\n"
	@printf "  $(BLUE)make check_requirements$(END) $(INFO) Verify required tools are installed\n"
	@printf "  $(BLUE)make help$(END)             $(INFO) Show this help message\n\n"
	@printf "$(INFO) Build artifacts:\n"
	@printf "  - $(KERNEL): Kernel binary (Multiboot compliant)\n"
	@printf "  - $(LINKER): Linker script (defines memory layout)\n"
	@printf "  - $(ISO): Bootable ISO image for QEMU/VM\n"
	@printf "  - $(GRUBCFG): GRUB configuration file\n"

################################################################################
#                               REQUIREMENTS CHECK                             #
# Verifies that all required tools are installed before building               #
################################################################################
check_requirements:
	@failed=false; \
	for tool in $(REQUIRED_TOOLS); do \
		if command -v $$tool >/dev/null 2>&1; then \
			echo "$(GREEN)$(LIGHT)[$$tool is installed.]$(END) Proceeding..."; \
		else \
			echo "$(RED)$(LIGHT)[$$tool is not installed.]$(END) Please install $$tool to proceed."; \
			failed=true; \
		fi; \
	done; \
	if [ "$$failed" = true ]; then exit 1; fi
	@if [ ! -f $(LINKER) ]; then \
		printf "$(ERROR) Linker script $(LINKER) not found!\n"; \
		printf "$(INFO) Create a linker.ld file in the project root\n"; \
		exit 1; \
	fi

################################################################################
#                               DIRECTORY CREATION                             #
# Creates build directories (.obj and .dep) with proper subdirectories         #
################################################################################
dir:
	@for DIR in $(DIRS_TO_CREATE); do \
		if [ ! -d $$DIR ]; then \
			mkdir -p $$DIR; \
			$(foreach SUB, $(SUB_DIRECTORIES), mkdir -p $$DIR/$(SUB);) \
			printf "$(BLUE)$(LIGTH)Creating directory:$(END) $$DIR\n"; \
		else \
			printf "$(BLUE)$(LIGTH)Directory already exists:$(END) $$DIR\n"; \
		fi \
	done

################################################################################
#                               ASSEMBLY COMPILATION RULE                      #
# Pattern rule for compiling .s files to .o files                              #
################################################################################
$(DIRECTORY_OBJ)/%.o: %.s Makefile
	@printf "$(INFO) Compiling $< ... \n"
	@if $(ASM) $(FLAGS_ASM) $(DIRECTORY_DEP)/$*.d $< -o $@; then \
		printf "$(SUCCESS) Object generated: $@\n"; \
	else \
		printf "$(ERROR) Failed to compile: $<\n"; \
	fi

################################################################################
#                               C COMPILATION RULE                             #
# Pattern rule for compiling .c files to .o files                              #
################################################################################
$(DIRECTORY_OBJ)/%.o: %.c Makefile
	@printf "$(INFO) Compiling $< ... \n"
	@if $(CC) $(CFLAGS) -MMD -MF $(DIRECTORY_DEP)/$*.d -c $< -o $@; then \
		printf "$(SUCCESS) Object generated: $@\n"; \
	else \
		printf "$(ERROR) Failed to compile: $<\n"; \
	fi

################################################################################
#                               KERNEL LINKING                                 #
# Links all object files into a single kernel binary using the linker script   #
# The linker script (linker.ld) defines memory layout and section placement    #
################################################################################
kernel: $(KERNEL)

$(KERNEL): $(OBJECTS) $(OBJECTS_C) $(LINKER)
	@printf "$(INFO) Linking kernel binary: $(KERNEL) ...\n"
	@printf "$(INFO) Using linker script: $(LINKER)\n"
	@if $(LD) $(LDFLAGS) -o $@ $(OBJECTS) $(OBJECTS_C); then \
		printf "$(SUCCESS) Created kernel: $(KERNEL)\n"; \
		printf "$(INFO) Kernel size: "; \
		wc -c < $(KERNEL) | awk '{printf "%d bytes (%.2f KB)\n", $$1, $$1/1024}'; \
		printf "$(INFO) Sections in $(KERNEL):\n"; \
		i686-elf-objdump -h $(KERNEL) 2>/dev/null | grep -E "^\s*[0-9]+\s+\.[a-zA-Z]" || true; \
	else \
		printf "$(ERROR) Failed to link kernel\n"; \
		exit 1; \
	fi

################################################################################
#                               GRUB CONFIGURATION                             #
# Creates a minimal GRUB configuration file for Multiboot compliance           #
################################################################################
$(GRUBCFG):
	@printf "$(INFO) Creating GRUB configuration: $(GRUBCFG) ...\n"
	@echo 'set timeout=0' > $(GRUBCFG)
	@echo 'set default=0' >> $(GRUBCFG)
	@echo '' >> $(GRUBCFG)
	@echo 'menuentry "kfs_1" {' >> $(GRUBCFG)
	@echo '    multiboot /boot/$(KERNEL)' >> $(GRUBCFG)
	@echo '    boot' >> $(GRUBCFG)
	@echo '}' >> $(GRUBCFG)
	@printf "$(SUCCESS) Created: $(GRUBCFG)\n"

################################################################################
#                               ISO CREATION                                   #
# Creates a bootable ISO image using GRUB's mkrescue tool                      #
# The ISO can be booted in QEMU, VirtualBox, or burned to physical media       #
################################################################################

iso: $(ISO)

$(ISO): $(KERNEL) $(GRUBCFG)
	@printf "$(INFO) Creating bootable ISO: $(ISO) ...\n"
	@rm -rf iso 2>/dev/null || true
	@mkdir -p iso/boot/grub
	@cp $(KERNEL) iso/boot/
	@cp $(GRUBCFG) iso/boot/grub/
	@printf "$(INFO) Running grub-mkrescue...\n"
	@if grub-mkrescue -o $(ISO) iso 2>&1; then \
		printf "$(SUCCESS) grub-mkrescue command completed\n"; \
	else \
		printf "$(WARNING) grub-mkrescue failed, trying alternative method...\n"; \
		if which xorriso >/dev/null 2>&1; then \
			xorriso -as mkisofs -R -b boot/grub/eltorito.img -no-emul-boot -boot-load-size 4 -boot-info-table -o $(ISO) iso 2>/dev/null && \
			printf "$(SUCCESS) ISO created with xorriso\n"; \
		elif which genisoimage >/dev/null 2>&1; then \
			genisoimage -R -b boot/grub/eltorito.img -no-emul-boot -boot-load-size 4 -boot-info-table -o $(ISO) iso 2>/dev/null && \
			printf "$(SUCCESS) ISO created with genisoimage\n"; \
		else \
			printf "$(ERROR) Failed to create ISO - no tools available\n"; \
			exit 1; \
		fi; \
	fi
	@rm -rf iso
	@if [ -f $(ISO) ]; then \
		printf "$(SUCCESS) ISO file created: $(ISO)\n"; \
		printf "$(INFO) ISO size: "; \
		ls -lh "$(ISO)" | awk '{print $$5}'; \
		printf "$(INFO) File type: "; \
		file "$(ISO)" 2>/dev/null | cut -d: -f2- || echo "ISO image"; \
	else \
		printf "$(ERROR) No ISO file was created\n"; \
		exit 1; \
	fi

################################################################################
#                               QEMU EXECUTION                                 #
# Runs the kernel in QEMU emulator for testing                                 #
# -cdrom: Boot from ISO image                                                  #
# -no-reboot: Exit instead of rebooting on shutdown                            #
# -no-shutdown: Keep QEMU running after kernel halts                           #
################################################################################
run: $(ISO)
	@printf "$(INFO) Starting QEMU with kernel ...\n"
	@printf "$(INFO) Press Ctrl-A then X to exit QEMU\n"
	@qemu-system-x86_64 -cdrom $(ISO) -no-reboot -no-shutdown

################################################################################
#                               DEBUGGING                                      #
# Runs QEMU with GDB stub enabled for kernel debugging                         #
# -s: Shorthand for -gdb tcp::1234 (wait for GDB connection on port 1234)      #
# -S: Freeze CPU at startup (wait for 'continue' from GDB)                     #
################################################################################
debug: $(ISO)
	@printf "$(INFO) Starting QEMU with GDB support ...\n"
	@printf "$(INFO) Connect GDB with: gdb -ex 'target remote localhost:1234' -ex 'symbol-file $(KERNEL)'\n"
	@printf "$(INFO) Press Ctrl-C in this terminal to stop QEMU\n"
	@qemu-system-x86_64 -cdrom $(ISO)

################################################################################
#                               CLEANUP TARGETS                                #
# clean: Removes intermediate build files                                      #
# distclean: Removes all generated files including final binaries              #
################################################################################


clean:
	@for DIR in $(DIRS_TO_CREATE); do \
		if [ -d $$DIR ]; then \
			$(RMV) $$DIR; \
			printf "$(SUCCESS) Removed directory: $$DIR\n"; \
		else \
			printf "$(WARNING) Directory not found: $$DIR\n"; \
		fi \
	done

	@printf "$(SUCCESS) Object and dependency files cleaned.\n"

distclean: clean
	@printf "$(INFO) Cleaning generated files ...\n"
	@for FILE in $(KERNEL) $(ISO) $(GRUBCFG); do \
		if [ -f "$$FILE" ]; then \
			$(RMV) "$$FILE"; \
			printf "$(SUCCESS) Removed: $$FILE\n"; \
		fi \
	done

################################################################################
#                               REBUILD TARGET                                 #
# Cleans everything and rebuilds from scratch                                  #
################################################################################
re: distclean all

################################################################################
#                               DEPENDENCY INCLUSION                           #
# Includes auto-generated dependency files for proper incremental builds        #
################################################################################
-include $(DEPENDENCIES)
-include $(DEPENDENCIES_C)

.SILENT:
################################################################################
#                               BUILD NOTES                                    #
# Additional setup instructions for cross-compiler toolchain                   #
################################################################################

# Note 1: Install cross-compiler dependencies
#   sudo apt install build-essential bison flex libgmp3-dev libmpc-dev \
#        libmpfr-dev texinfo libisl-dev

# Note 2: Set up cross-compiler environment variables
#   export PREFIX="$HOME/opt/cross"
#   export TARGET=i686-elf
#   export PATH="$PREFIX/bin:$PATH"
#
#   mkdir -p $HOME/src
#   cd $HOME/src

# Note 3: Build binutils
#   wget https://ftp.gnu.org/gnu/binutils/binutils-2.42.tar.gz
#   tar xf binutils-2.42.tar.gz
#   mkdir build-binutils
#   cd build-binutils
#   ../binutils-2.42/configure --target=$TARGET --prefix="$PREFIX" \
#        --with-sysroot --disable-nls --disable-werror
#   make
#   make install
#   cd ..

# Note 4: Build GCC
#   wget https://ftp.gnu.org/gnu/gcc/gcc-13.2.0/gcc-13.2.0.tar.gz
#   tar xf gcc-13.2.0.tar.gz
#   mkdir build-gcc
#   cd build-gcc
#   ../gcc-13.2.0/configure --target=$TARGET --prefix="$PREFIX" \
#        --disable-nls --enable-languages=c,c++ --without-headers
#   make all-gcc
#   make all-target-libgcc
#   make install-gcc
#   make install-target-libgcc

# Note 5: Add to ~/.bashrc for permanent setup
#   export PREFIX="$HOME/opt/cross"
#   export TARGET=i686-elf
#   export PATH="$PREFIX/bin:$PATH"
