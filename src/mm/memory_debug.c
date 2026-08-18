// SPDX-License-Identifier: GPL-2.0

/**
 * @file memory_debug.c
 * @brief Memory allocation debugging and tracking
 */

#include <memory.h>
#include <pmm.h>
#include <print.h>

static uint32_t alloc_count = 0;
static uint32_t free_count = 0;
static uint32_t total_allocated = 0;
static uint32_t total_freed = 0;

void *debug_vmalloc(size_t size, const char *caller)
{
	void *ptr = vmalloc(size);
	if (ptr) {
		alloc_count++;
		total_allocated += size;
		printf("[VMALLOC] %s: %u bytes at 0x%x (total: %u bytes, %u allocs)\n",
			caller, (uint32_t)size, (uint32_t)ptr, total_allocated, alloc_count);
	} else {
		printf("[VMALLOC ERROR] %s: failed to allocate %u bytes\n",
			caller, (uint32_t)size);
	}
	return ptr;
}

void debug_vfree(void *ptr, const char *caller)
{
	size_t size;
	
	if (!ptr) {
		printf("[VFREE WARNING] %s: attempted to free NULL\n", caller);
		return;
	}

	size = vsize(ptr);
	vfree(ptr);
	free_count++;
	total_freed += size;
	printf("[VFREE] %s: freed 0x%x (%u bytes, total freed: %u bytes, %u frees)\n",
		caller, (uint32_t)ptr, (uint32_t)size, total_freed, free_count);
}

void memory_debug_print_stats(void)
{
	uint32_t net = total_allocated - total_freed;
	printf("\n=== Memory Debug Stats ===\n");
	printf("Allocations:   %u\n", alloc_count);
	printf("Frees:         %u\n", free_count);
	printf("Total alloc:   %u bytes\n", total_allocated);
	printf("Total freed:   %u bytes\n", total_freed);
	printf("Net in use:    %u bytes\n", net);
	printf("PMM free pages: %u\n", (uint32_t)pmm_get_free_frame_count());
	printf("PMM used pages: %u\n", (uint32_t)pmm_get_used_frame_count());
	printf("==========================\n\n");
}
