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
	if (magic != 0x2BADB002)
		return -1;

	if (!info || !info->mmap_addr || !info->mmap_length)
		return -1;

	init_system();
	print_multiboot_info(info);
	printf("\033[1;31mRed\033[m \033[1;32mGreen\033[m \033[1;34mBlue\033[m \033[1;33mYellow\033[m\n");
	sys.main_loop(&sys);
	return 0;
}
