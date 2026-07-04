// SPDX-License-Identifier: GPL-2.0
#include <pmm.h>
#include <stack_kernel.h>

static uint8_t *bitmap = NULL;
static uint32_t bitmap_size = 0;
static uint32_t total_pages = 0;
static uint32_t used_pages = 0;
static uint32_t memory_base = 0x100000;
static uint32_t first_free_hint = 0;

/**
 * @brief Set a bit in the bitmap to mark a page as used.
 *
 * This function sets the bit corresponding to the given page index in the
 * bitmap, indicating that the page is now used. It also increments the count
 * of used pages.
 *
 * @param bit The index of the page to mark as used.
 */
static inline void bitmap_set_bit(uint32_t bit) {
	bitmap[bit / 8] |= (1 << (bit % 8));
}

/**
 * @brief Clear a bit in the bitmap to mark a page as free.
 *
 * This function clears the bit corresponding to the given page index in the
 * bitmap, indicating that the page is now free. It also decrements the count
 * of used pages.
 *
 * @param bit The index of the page to mark as free.
 */
static inline void bitmap_clear_bit(uint32_t bit) {
	bitmap[bit / 8] &= ~(1 << (bit % 8));
}

/**
 * @brief Check if a bit in the bitmap is set (used).
 *
 * This function checks if the bit corresponding to the given page index in the
 * bitmap is set, indicating that the page is currently used.
 *
 * @param bit The index of the page to check.
 * @return true if the page is used, false otherwise.
 */
static inline int bitmap_test_bit(uint32_t bit) {
	return (bitmap[bit / 8] >> (bit % 8)) & 1;
}

/**
 * @brief Find the first free page starting from a given index.
 *
 * This function searches for the first free page in the bitmap, starting from
 * the specified index.
 *
 * @param start The index to start searching from.
 * @return The index of the first free page, or -1 if no free page is found.
 */
static int bitmap_find_first_free(uint32_t start) {
	uint32_t byte_idx = start / 8;
	uint32_t bit_idx = start % 8;
	
	for (uint32_t i = byte_idx; i < (total_pages + 7) / 8; i++) {
		uint8_t byte = bitmap[i];

		if (byte != 0xFF) {
			uint32_t start_bit = (i == byte_idx) ? bit_idx : 0;

			for (uint32_t j = start_bit; j < 8; j++) {
				uint32_t bit_num = i * 8 + j;

				if (bit_num >= total_pages)
					return -1;
				if (!(byte & (1 << j))) {
					return bit_num;
				}
			}
		}
	}
	return -1;
}

static int bitmap_find_first_free_range(uint32_t start, size_t count)
{
	if (!count)
		return -1;

	for (uint32_t page = start; page + count <= total_pages; page++) {
		size_t free_count = 0;

		while (free_count < count &&
			!bitmap_test_bit(page + free_count)) {
			free_count++;
		}

		if (free_count == count)
			return (int)page;

		page += free_count;
	}

	return -1;
}

void pmm_init(multiboot_info_t *info) {

	printf("Initializing Page Frame Allocator (Bitmap)...\n");
	
	// ============================================================
	// 1. Verify memory map is available
	// ============================================================
	if (!info || !(info->flags & MULTIBOOT_INFO_MEMORY)) {
		printf("ERROR: No memory map available\n");
		return;
	}
	
	// Print memory map (for debugging)
	print_multiboot_info(info);
	
	// ============================================================
	// 2. Calculate total usable RAM
	// ============================================================
	multiboot_map_entry_t *entry = (multiboot_map_entry_t*)info->mmap_addr;
	uint64_t max_addr = 0;
	uint64_t total_ram = 0;
	
	for (; (uint32_t)entry < info->mmap_addr + info->mmap_length;
		entry = (multiboot_map_entry_t*)((uint32_t)entry + entry->size + 4)) {
		
		if (entry->type == 1) {  // Usable RAM
			uint64_t base = entry->base_addr;
			uint64_t length = entry->length;
			uint64_t end = base + length;
			
			if (end > max_addr) max_addr = end;
			total_ram += length;
		}
	}
	
	// ============================================================
	// 3. Calculate total number of pages
	// ============================================================
	total_pages = (max_addr + PAGE_SIZE - 1) / PAGE_SIZE;
	bitmap_size = (total_pages + 7) / 8;  // Round up
	
	printf("Total memory: %d MB\n", (uint32_t)(total_ram / (1024 * 1024)));
	printf("Total pages: %d\n", total_pages);
	printf("Bitmap size: %d bytes (%d KB)\n", bitmap_size, bitmap_size / 1024);
	
	// ============================================================
	// 4. Get kernel end address (from linker)
	// ============================================================
	extern uint32_t endkernel;
	uint32_t kernel_end = (uint32_t)&endkernel;
	
	// Align to page (4 KB)
	uint32_t bitmap_phys = (kernel_end + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);
	bitmap = (uint8_t*)bitmap_phys;
	
	printf("Kernel ends at:  0x%x\n", kernel_end);
	printf("Bitmap at:       0x%x\n", (uint32_t)bitmap);
	printf("Bitmap occupies: %d pages\n", (bitmap_size + PAGE_SIZE - 1) / PAGE_SIZE);
	
	// ============================================================
	// 5. Initialize bitmap (all bits 0 = free)
	// ============================================================
	ft_memset(bitmap, 0, bitmap_size);
	used_pages = 0;
	first_free_hint = 0;
	
	// ============================================================
	// 6. Mark bitmap pages as used
	// ============================================================
	uint32_t bitmap_pages = (bitmap_size + PAGE_SIZE - 1) / PAGE_SIZE;
	for (uint32_t i = 0; i < bitmap_pages; i++) {
		uint32_t page_num = (bitmap_phys / PAGE_SIZE) + i;
		if (page_num < total_pages) {
			bitmap_set_bit(page_num);
			used_pages++;
		}
	}
	
	// ============================================================
	// 7. Mark kernel pages as used
	// ============================================================
	// Kernel starts at memory_base (1 MB) and ends at kernel_end
	uint32_t kernel_start_page = memory_base / PAGE_SIZE;
	uint32_t kernel_end_page = (kernel_end + PAGE_SIZE - 1) / PAGE_SIZE;
	
	for (uint32_t i = kernel_start_page; i < kernel_end_page; i++) {
		if (i < total_pages && !bitmap_test_bit(i)) {
			bitmap_set_bit(i);
			used_pages++;
		}
	}
	
	// ============================================================
	// 8. Mark reserved regions as used
	// ============================================================
	entry = (multiboot_map_entry_t*)info->mmap_addr;
	for (; (uint32_t)entry < info->mmap_addr + info->mmap_length;
		entry = (multiboot_map_entry_t*)((uint32_t)entry + entry->size + 4)) {
		
		if (entry->type != 1) {  // Not usable RAM
			uint64_t base = entry->base_addr;
			uint64_t length = entry->length;
			uint32_t start_page = base / PAGE_SIZE;
			uint32_t end_page = (base + length + PAGE_SIZE - 1) / PAGE_SIZE;
			
			for (uint32_t i = start_page; i < end_page && i < total_pages; i++) {
				if (!bitmap_test_bit(i)) {
					bitmap_set_bit(i);
					used_pages++;
				}
			}
		}
	}
	
	// ============================================================
	// 9. Final statistics
	// ============================================================
	printf("\n=== PMM Initialized ===\n");
	printf("Total pages:    %d\n", total_pages);
	printf("Used pages:     %d (%.2f%%)\n", used_pages, 
		(float)used_pages / total_pages * 100);
	printf("Free pages:     %d (%.2f%%)\n", total_pages - used_pages,
		(float)(total_pages - used_pages) / total_pages * 100);
	printf("Free memory:    %d MB\n", ((total_pages - used_pages) * PAGE_SIZE) / (1024 * 1024));
	printf("=======================\n\n");
}

// ============================================================
// ALLOCATION AND DEALLOCATION FUNCTIONS
// ============================================================

/**
 * @brief Allocate a physical page frame.
 * @return Physical address of allocated page, or 0 if out of memory.
 */
uint32_t pmm_alloc_frame(void) {
	// Find first free bit from hint
	int page_num = bitmap_find_first_free(first_free_hint);
	
	if (page_num == -1) {
		// If not found from hint, search from beginning
		page_num = bitmap_find_first_free(0);
		if (page_num == -1) {
			printf("ERROR: No free pages available\n");
			return 0;  // Out of memory
		}
	}
	
	// Mark page as used
	bitmap_set_bit(page_num);
	used_pages++;
	
	// Update hint (to speed up future searches)
	first_free_hint = page_num + 1;
	
	// Return physical address
	uint32_t phys_addr = memory_base + (page_num * PAGE_SIZE);
	return phys_addr;
}

/**
 * @brief Allocate a contiguous range of physical page frames.
 *
 * @param count Number of contiguous pages to allocate.
 * @return Physical address of the first page, or 0 if no range is available.
 */
uint32_t pmm_alloc_frame_range(size_t count) {
	int page_num;

	if (!count)
		return 0;

	page_num = bitmap_find_first_free_range(first_free_hint, count);
	if (page_num == -1) {
		page_num = bitmap_find_first_free_range(0, count);
		if (page_num == -1)
			return 0;
	}

	for (size_t i = 0; i < count; i++) {
		bitmap_set_bit((uint32_t)page_num + i);
		used_pages++;
	}

	first_free_hint = (uint32_t)page_num + (uint32_t)count;
	return memory_base + ((uint32_t)page_num * PAGE_SIZE);
}

/**
 * @brief Free a physical page frame.
 * @param phys_addr Physical address of page to free.
 */
void pmm_free_frame(uint32_t phys_addr) {
	// Check that address is aligned
	if (phys_addr % PAGE_SIZE != 0) {
		printf("ERROR: Unaligned address: 0x%x\n", phys_addr);
		return;
	}
	
	// Calculate page number
	if (phys_addr < memory_base) {
		printf("ERROR: Address below memory_base: 0x%x\n", phys_addr);
		return;
	}
	
	uint32_t page_num = (phys_addr - memory_base) / PAGE_SIZE;
	
	if (page_num >= total_pages) {
		printf("ERROR: Page out of range: %d (max: %d)\n", page_num, total_pages);
		return;
	}
	
	// Check that page was used
	if (!bitmap_test_bit(page_num)) {
		printf("WARNING: Attempting to free already free page: 0x%x\n", phys_addr);
		return;
	}
	
	// Free the page
	bitmap_clear_bit(page_num);
	used_pages--;
	
	// Update hint if necessary
	if (page_num < first_free_hint) {
		first_free_hint = page_num;
	}
}

/**
 * @brief Free a range of physical pages.
 * @param phys_addr Starting physical address.
 * @param count Number of pages to free.
 */
void pmm_free_frame_range(uint32_t phys_addr, size_t count) {
	for (size_t i = 0; i < count; i++) {
		pmm_free_frame(phys_addr + (i * PAGE_SIZE));
	}
}

// ============================================================
// INFORMATION AND STATISTICS FUNCTIONS
// ============================================================

size_t pmm_get_free_frame_count(void) {
	return total_pages - used_pages;
}

size_t pmm_get_total_frame_count(void) {
	return total_pages;
}

size_t pmm_get_used_frame_count(void) {
	return used_pages;
}

void pmm_print_stats(void) {
	printf("\n=== Physical Memory Statistics ===\n");
	printf("Total pages:   %d\n", total_pages);
	printf("Used pages:    %d\n", used_pages);
	printf("Free pages:    %d\n", total_pages - used_pages);
	printf("Free memory:   %d KB\n", (total_pages - used_pages) * 4);
	printf("Bitmap at:     0x%x (%d bytes)\n", (uint32_t)bitmap, bitmap_size);
	printf("===================================\n\n");
}

/**
 * @brief Print bitmap state (for debugging).
 * Shows first 64 bits and first used/free bit.
 */
void pmm_print_bitmap(void) {
	printf("Bitmap (first 64 bits): ");
	for (uint32_t i = 0; i < 64 && i < total_pages; i++) {
		printf("%d", bitmap_test_bit(i));
		if ((i + 1) % 8 == 0) printf(" ");
		printf("\n");
	}
	printf("\n");
	
	printf("First pages: ");
	for (uint32_t i = 0; i < 16 && i < total_pages; i++) {
		printf("0x%x ", memory_base + (i * PAGE_SIZE));
		printf("[%s] ", bitmap_test_bit(i) ? "U" : "F");
		printf("\n");
	}
	printf("\n");
}
