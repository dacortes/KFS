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
#include <pmm.h>
#include <memory.h>

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
	if (magic != MULTIBOOT_MAGIC)
		return -1;

	if (!info || !info->mmap_addr || !info->mmap_length)
		return -1;

	init_system();
	pmm_init(info);
	memory_init();
	printf("[BOOT] memory helpers ready\n");
	sys.main_loop(&sys, &info);
	return 0;
}
