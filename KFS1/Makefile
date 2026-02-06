################################################################################
#                               COLORS                                         #
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
#                               LABELS                                         #
################################################################################

SUCCESS = $(LIGTH)$(GREEN)[SUCCESS]$(END)
WARNING = $(LIGTH)$(YELLOW)[WARNING]$(END)
INFO = $(LIGTH)$(BLUE)[INFO]$(END)
ERROR = $(LIGTH)$(RED)[ERROR]$(END)

################################################################################
#                               VARIABLES                                      #
################################################################################

RMV = rm -rf

REQUIRED_TOOLS := qemu-system-x86_64 nasm gcc grub-mkrescue

ASM := nasm -f elf32 -w+all -w+error -w+orphan-labels -g
FLAGS_ASM := -MD -MF
CC = i686-elf-gcc
LD = i686-elf-ld
CFLAGS = -m32 -ffreestanding -O2 -Wall -Wextra -fno-builtin -nostdlib
LDFLAGS = -m elf_i386

SOURCES_AS := \
	boot/entry.s


SOURCES_C := \
	kernel/main.c

DIRECTORY_OBJ = .obj
DIRECTORY_DEP = .dep
OBJECTS = $(addprefix $(DIRECTORY_OBJ)/, $(SOURCES_AS:.s=.o))
DEPENDENCIES = $(addprefix $(DIRECTORY_DEP)/, $(SOURCES_AS:.s=.d))

OBJECTS_C = $(addprefix $(DIRECTORY_OBJ)/, $(SOURCES_C:.c=.o))
DEPENDENCIES_C = $(addprefix $(DIRECTORY_DEP)/, $(SOURCES_C:.c=.d))

DIRS_TO_CREATE = $(DIRECTORY_OBJ) $(DIRECTORY_DEP)
SUB_DIRECTORIES := boot kernel

all: check_requirements dir $(OBJECTS)

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

$(DIRECTORY_OBJ)/%.o: %.s Makefile
	@printf "$(INFO) Compiling $< ... \n"
	@if $(ASM) $(FLAGS_ASM) $(DIRECTORY_DEP)/$*.d $< -o $@; then \
		printf "$(SUCCESS) Object generated: $@\n"; \
	else \
		printf "$(ERROR) Failed to compile: $<\n"; \
	fi

$(DIRECTORY_OBJ)/%.o: %.c Makefile
	@printf "$(INFO) Compiling $< ... \n"
	@if $(CC) $(CFLAGS) -MMD -MF $(DIRECTORY_DEP)/$*.d -c $< -o $@; then \
		printf "$(SUCCESS) Object generated: $@\n"; \
	else \
		printf "$(ERROR) Failed to compile: $<\n"; \
	fi

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

hola: $(OBJECTS_C)

re: clean all

-include $(DEPENDENCIES)
-include $(DEPENDENCIES_C)

.SILENT:

#Nota:
# Se requier instalar i686-elf-gcc para el debido compilamiento y usos de las flags
# tiene dependeccias de este paquete sudo apt-get install gcc-13-i686-linux-gnu