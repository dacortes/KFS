################################################################################
#                               BOLD COLORS                                    #
################################################################################

END = \033[m
RED = \033[31m
GREEN = \033[32m
YELLOW = \033[33m
BLUE = \033[34m
PURPLE = \033[35m
CIAN = \033[36m

################################################################################
#  FONT                                                                        #
################################################################################

LIGTH = \033[1m
DARK = \033[2m
ITALIC = \033[3m

################################################################################
#                               VARIABLES                                      #
################################################################################


REQUIRED_TOOLS := qemu-system-x86_64 nasm gcc

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
