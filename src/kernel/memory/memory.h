// SPDX-License-Identifier: GPL-2.0

/**
 * @file memory.h
 * @brief Physical page frame management for the kernel.
 *
 * Provides page metadata, bitmap-backed page tracking, and helpers
 * to initialize the memory map from Multiboot information.
 */

#pragma once

#include <stdint.h>
#include <multiboot.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MEMORY_PAGE_SIZE		4096U
#define MEMORY_MAX_PHYSICAL_MEMORY	(256U * 1024U * 1024U)
#define MEMORY_MAX_PAGES		(MEMORY_MAX_PHYSICAL_MEMORY / \
					 MEMORY_PAGE_SIZE)
#define MEMORY_BITMAP_SIZE		(MEMORY_MAX_PAGES / 8U)
#define MEMORY_KERNEL_RESERVED_LIMIT	0x00200000U

#define MEMORY_PAGE_FLAG_FREE		0x01U
#define MEMORY_PAGE_FLAG_RESERVED	0x02U

/**
 * @struct memory_page_s
 * @brief Metadata tracked for a single physical page frame.
 */
typedef struct memory_page_s {
	uint32_t index;
	uint32_t address;
	uint32_t flags;
} memory_page_t;

/**
 * Initialize the physical page allocator from the Multiboot memory map.
 *
 * The allocator builds an internal page bitmap, marks unusable memory
 * as reserved, and exposes helper functions for page lookup and page
 * creation.
 *
 * @param info Multiboot information structure provided by GRUB
 * @return 0 on success, -1 on failure
 *
 * @note Must be called before any page lookup or page creation helper.
 */
int memory_init(multiboot_info_t *info);

/**
 * Get the metadata entry for the page containing an address.
 *
 * The address is aligned down to the nearest 4 KiB page boundary before
 * the page lookup is performed.
 *
 * @param address Physical address to resolve
 * @return Pointer to the page metadata, or NULL if out of range
 */
memory_page_t *memory_get_page(uint32_t address);

/**
 * Create or claim a page for a physical address.
 *
 * If the page is free, it is marked as allocated. If the page is already
 * allocated, the existing page descriptor is returned. Reserved pages are
 * never created and return NULL.
 *
 * @param address Physical address to claim
 * @return Pointer to the page metadata, or NULL if the page is reserved
 *         or outside the managed range
 */
memory_page_t *memory_create_page(uint32_t address);

/**
 * Allocate the first free physical page.
 *
 * @return Pointer to the first free page, or NULL if no free page exists
 */
memory_page_t *memory_allocate_page(void);

/**
 * Release an allocated physical page back to the bitmap.
 *
 * Reserved pages remain untouched.
 *
 * @param address Physical address to release
 */
void memory_free_page(uint32_t address);

/**
 * Get the number of managed physical pages.
 *
 * @return Total number of tracked pages
 */
uint32_t memory_get_total_pages(void);

/**
 * Get the number of free physical pages.
 *
 * @return Number of free pages currently available
 */
uint32_t memory_get_free_pages(void);

/**
 * Get the memory page size used by the allocator.
 *
 * @return Page size in bytes
 */
uint32_t memory_get_page_size(void);

/**
 * Print a compact summary of the current physical memory state.
 *
 * The output includes the page size, total and free page counts, and a
 * short preview of tracked page states.
 *
 * @note Intended for shell/debugger use.
 */
void memory_dump_state(void);

#ifdef __cplusplus
}
#endif
