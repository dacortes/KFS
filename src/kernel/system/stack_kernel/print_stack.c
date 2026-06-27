// SPDX-License-Identifier: GPL-2.0
#include <stack_kernel.h>

/**
 * @brief Print `n` spaces efficiently.
 */
void print_spaces(int n)
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
void print_pad_right(const char *s, int width)
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
	multiboot_map_entry_t *entry = (multiboot_map_entry_t*)info->mmap_addr;
	uint64_t total_usable = 0;
	uint64_t total_reserved = 0;
	
	printf("\n=== Memory Map (from GRUB) ===\n");
	print_pad_right("Type", 8);
	print_pad_right("| Start", 11);
	print_pad_right("| End", 11);
	print_pad_right("| Size", 11);
	printf("\n");
	printf("==================================================\n");

	
	for (; (uint32_t)entry < info->mmap_addr + info->mmap_length;
		entry = (multiboot_map_entry_t*)((uint32_t)entry + entry->size + 4)) {
		
		uint64_t base = entry->base_addr;
		uint64_t length = entry->length;
		uint64_t end = base + length;
		
		const char *type_str;
		const char *color;

		if (entry->type == 1) {
			type_str = "Usable";
			color = "\033[32m";  // Green
			total_usable += length;
		} else if (entry->type == 2) {
			type_str = "Reserved";
			color = "\033[33m";  // Yellow
			total_reserved += length;
		} else if (entry->type == 3) {
			type_str = "ACPI Recl.";
			color = "\033[34m";  // Blue
			total_reserved += length;
		} else if (entry->type == 4) {
			type_str = "ACPI NVS";
			color = "\033[35m";  // Magenta
			total_reserved += length;
		} else {
			type_str = "Unknown";
			color = "\033[31m";  // Red
			total_reserved += length;
		}
		int bytes_p = printf("%s%s\033[m", color, type_str);

		print_spaces(17 - bytes_p);
		bytes_p = printf("%x", (uint32_t)base);
		print_spaces(11 - bytes_p);
		bytes_p = printf("%x", (uint32_t)end);
		print_spaces(11 - bytes_p);
		bytes_p = printf("%d KB", (uint32_t)(length / (1024 * 1024)));
		print_spaces(11 - bytes_p);
		printf("%s\n", (entry->type == 1) ? "\033[32mFree\033[m" : "\033[31mReserved\033[m");
	}
	
	printf("==================================================\n");
	printf("Total usable:    %d MB\n", (uint32_t)(total_usable / (1024 * 1024)));
	printf("Total reserved:  %d MB\n", (uint32_t)(total_reserved / (1024 * 1024)));
	printf("Total RAM:       %d MB\n", (uint32_t)((total_usable + total_reserved) / (1024 * 1024)));
	printf("==================================================\n");
}
