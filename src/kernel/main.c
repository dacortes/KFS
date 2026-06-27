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

extern uint32_t text_start;
extern uint32_t rodata_start;
extern uint32_t data_start;
extern uint32_t bss_start;
extern uint32_t endkernel;
extern uint32_t kernel_start;
extern uint32_t kernel_size;
extern uint32_t stack_top;
extern uint32_t stack_bottom;

int kernel_main(uint32_t magic, multiboot_info_t *info)
{
	if (magic != MULTIBOOT_MAGIC)
		return -1;

	if (!info || !info->mmap_addr || !info->mmap_length)
		return -1;

	init_system();
	pmm_init(info);
	// printf("=== Testeando Page Frame Allocator ===\n");
	
	// // Asignar 10 páginas
	// uint32_t pages[10];
	// printf("Asignando 10 páginas:\n");
	// for (int i = 0; i < 10; i++) {
	// 	pages[i] = pmm_alloc_frame();
	// 	printf("  Página %d: 0x%x\n", i, pages[i]);
	// }
	
	// pmm_print_stats();
	
	// // Liberar algunas páginas (3, 5, 7)
	// printf("\nLiberando páginas 3, 5, 7...\n");
	// pmm_free_frame(pages[3]);
	// pmm_free_frame(pages[5]);
	// pmm_free_frame(pages[7]);
	
	// pmm_print_stats();
	
	// // Asignar de nuevo (debería reutilizar las liberadas)
	// printf("\nAsignando 3 páginas nuevas (debería reutilizar):\n");
	// for (int i = 0; i < 3; i++) {
	// 	uint32_t new_page = pmm_alloc_frame();
	// 	printf("  Nueva página: 0x%x\n", new_page);
	// }
	
	// pmm_print_stats();
	// pmm_print_bitmap();
	
	// printf("\n=== PMM test completado ===\n");
	
	// printf("\nKernel listo ...\n");
	sys.main_loop(&sys, &info);
	return 0;
}
