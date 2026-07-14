// SPDX-License-Identifier: GPL-2.0

#include <print.h>
#include <multiboot.h>

void print_spaces(int n);
void print_pad_right(const char *s, int width);
/**
 * @brief Print multiboot information.
 *
 * This function prints the multiboot information provided by the bootloader
 * to the console. It displays the base address, length, type, and status of
 * each memory region described in the multiboot memory map.
 *
 * @param info Pointer to the multiboot information structure.
 */
void print_multiboot_info(multiboot_info_t *info);
