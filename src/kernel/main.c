// SPDX-License-Identifier: GPL-2.0

/**
 * @file main.c
 * @brief KFS kernel entry point.
 *
 * The kernel entry is responsible for bootstrapping core subsystems
 * and transferring control to the system runtime loop.
 */

#include <system.h>
#include <print.h>
#include <multiboot.h>

/* Multiboot information structures and definitions. */

/**
 * @brief Print `n` spaces efficiently.
 */
static void print_spaces(int n)
{
	char buf[64];
	while (n > 0) {
		int take = n > (int)(sizeof(buf) - 1) ? (int)(sizeof(buf) - 1) : n;
		for (int i = 0; i < take; i++)
			buf[i] = ' ';
		buf[take] = '\0';
		printf("%s", buf);
		n -= take;
	}
}

/**
 * @brief Print string then pad to `width` with spaces (right-pad).
 */
static void print_pad_right(const char *s, int width)
{
	int len = ft_strlen(s);
	printf("%s", s);
	if (len < width)
		print_spaces(width - len);
}

void print_multiboot_info(multiboot_info_t *info)
{
	printf("\n");
	printf("================================================================================");
	print_pad_right("Base Address", 16);
	print_pad_right("Length (KB)", 16);
	print_pad_right("Type", 16);
	printf("Status\n");
	printf("================================================================================\n");

	multiboot_map_entry_t *mmap = (multiboot_map_entry_t *)info->mmap_addr;
	uint32_t mmap_end = info->mmap_addr + info->mmap_length;

	for (; (uint32_t)mmap < mmap_end; 
	mmap = (multiboot_map_entry_t *)((uint32_t)mmap + mmap->size + sizeof(mmap->size))) {
		uint32_t base_lo = (uint32_t)mmap->base_addr;
		uint32_t length_lo = (uint32_t)mmap->length;
		uint32_t length_kb = length_lo / 1024;
		const char *type_str;
		const char *status_str;
		const char *color;

		if (mmap->type == 1) {
			type_str = "Usable";
			status_str = "Free";
			color = "\033[1;32m";  /* Green */
		} else {
			type_str = "Reserved";
			status_str = "Used";
			color = "\033[1;31m";  /* Red */
		}

		int num_base = printf("0x%X", base_lo);
		if (num_base < 16)
			print_spaces(16 - num_base);
		int num_length = printf("%u", length_kb);
		if (num_length < 16)
			print_spaces(16 - num_length);
		print_pad_right(type_str, 16);
		printf("%s%s\033[m\n", color, status_str);
	}
	printf("================================================================================\n\n");
}

/**
 * @brief Kernel entry point.
 *
 * Calls `init_system()` to initialize kernel subsystems and then
 * transfers control to the runtime main loop defined by
 * `sys.main_loop`. Under normal operation this function does not
 * return.
 *
 * @return Zero on unexpected return (function generally does not return).
 */
int kernel_main(uint32_t magic, multiboot_info_t *info)
{
	if (magic != 0x2BADB002) {
		return -1;
	}

	if (!info || !info->mmap_addr || !info->mmap_length) {
		return -1;
	}

	init_system();
	print_multiboot_info(info);
	// read_multiboot_info(info);
	printf("\033[1;31mRed\033[m \033[1;32mGreen\033[m \033[1;34mBlue\033[m \033[1;33mYellow\033[m\n");
	sys.main_loop(&sys);
	return 0;
}
