// SPDX-License-Identifier: GPL-2.0
#include <stack_kernel.h>

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
/**
 * @brief Print multiboot information.
 *
 * This function prints the multiboot information provided by the bootloader
 * to the console. It displays the base address, length, type, and status of
 * each memory region described in the multiboot memory map.
 *
 * @param info Pointer to the multiboot information structure.
 */
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

	for ( ; (uint32_t)mmap < mmap_end;
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
			color = "\033[1;32m";
		} else {
			type_str = "Reserved";
			status_str = "Used";
			color = "\033[1;31m";
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
