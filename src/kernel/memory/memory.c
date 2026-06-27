// SPDX-License-Identifier: GPL-2.0

/**
 * @file memory.c
 * @brief Physical page frame allocator and page lookup helpers.
 *
 * The allocator uses a bitmap to track page state and a page metadata
 * table to expose the bookkeeping for each physical frame.
 */

#include <kernel/memory/memory.h>
#include <print.h>
#include <helper.h>

typedef struct memory_state_s {
	multiboot_info_t *boot_info;
	memory_page_t pages[MEMORY_MAX_PAGES];
	uint8_t bitmap[MEMORY_BITMAP_SIZE];
	uint32_t total_pages;
	uint32_t free_pages;
} memory_state_t;

static memory_state_t memory_state;

static uint32_t align_down(uint32_t address)
{
	return address & ~(MEMORY_PAGE_SIZE - 1U);
}

static uint32_t align_up(uint32_t address)
{
	return (address + MEMORY_PAGE_SIZE - 1U) & ~(MEMORY_PAGE_SIZE - 1U);
}

static void bitmap_set(uint32_t index)
{
	memory_state.bitmap[index / 8U] |= (uint8_t)(1U << (index % 8U));
}

static void bitmap_clear(uint32_t index)
{
	memory_state.bitmap[index / 8U] &= (uint8_t)~(1U << (index % 8U));
}

static int bitmap_test(uint32_t index)
{
	return (memory_state.bitmap[index / 8U] >> (index % 8U)) & 1U;
}

static void page_init(uint32_t index)
{
	memory_page_t *page;

	page = &memory_state.pages[index];
	page->index = index;
	page->address = index * MEMORY_PAGE_SIZE;
	page->flags = MEMORY_PAGE_FLAG_RESERVED;
	bitmap_set(index);
}

static void page_mark_free(uint32_t index)
{
	memory_page_t *page;

	page = &memory_state.pages[index];
	if (page->flags & MEMORY_PAGE_FLAG_FREE) {
		return;
	}

	page->flags = MEMORY_PAGE_FLAG_FREE;
	bitmap_clear(index);
	memory_state.free_pages++;
}

static void page_mark_reserved(uint32_t index)
{
	memory_page_t *page;

	page = &memory_state.pages[index];
	if (page->flags & MEMORY_PAGE_FLAG_FREE) {
		memory_state.free_pages--;
	}

	page->flags = MEMORY_PAGE_FLAG_RESERVED;
	bitmap_set(index);
}

static void page_mark_allocated(uint32_t index)
{
	memory_page_t *page;

	page = &memory_state.pages[index];
	if (page->flags & MEMORY_PAGE_FLAG_FREE) {
		memory_state.free_pages--;
	}

	page->flags = 0;
	bitmap_set(index);
}

static void mark_range_free(uint32_t start, uint32_t end)
{
	uint32_t index;

	if (start >= end) {
		return;
	}

	start = align_up(start);
	end = align_down(end);
	if (start >= end) {
		return;
	}

	for (index = start / MEMORY_PAGE_SIZE; index < end / MEMORY_PAGE_SIZE;
		index++) {
		if (index >= memory_state.total_pages) {
			break;
		}
		page_mark_free(index);
	}
}

static void mark_range_reserved(uint32_t start, uint32_t end)
{
	uint32_t index;

	if (start >= end) {
		return;
	}

	start = align_down(start);
	end = align_up(end);
	if (start >= end) {
		return;
	}

	for (index = start / MEMORY_PAGE_SIZE; index < end / MEMORY_PAGE_SIZE;
		index++) {
		if (index >= memory_state.total_pages) {
			break;
		}
		page_mark_reserved(index);
	}
}

static uint32_t compute_highest_address(multiboot_info_t *info)
{
	multiboot_map_entry_t *entry;
	uint32_t mmap_end;
	uint64_t highest;

	highest = MEMORY_PAGE_SIZE;
	entry = (multiboot_map_entry_t *)info->mmap_addr;
	mmap_end = info->mmap_addr + info->mmap_length;

	while ((uint32_t)entry < mmap_end) {
		uint64_t entry_end;

		entry_end = entry->base_addr + entry->length;
		if (entry_end > highest) {
			highest = entry_end;
		}

		entry = (multiboot_map_entry_t *)((uint32_t)entry +
			entry->size + sizeof(entry->size));
	}

	if (highest > MEMORY_MAX_PHYSICAL_MEMORY) {
		highest = MEMORY_MAX_PHYSICAL_MEMORY;
	}

	return align_up((uint32_t)highest);
}

static void reserve_bootloader_metadata(multiboot_info_t *info)
{
	uint32_t info_start;
	uint32_t info_end;
	uint32_t mmap_start;
	uint32_t mmap_end;

	info_start = (uint32_t)info;
	info_end = info_start + sizeof(*info);
	mark_range_reserved(info_start, info_end);

	mmap_start = info->mmap_addr;
	mmap_end = mmap_start + info->mmap_length;
	mark_range_reserved(mmap_start, mmap_end);
}

static void reserve_kernel_space(void)
{
	mark_range_reserved(0U, MEMORY_KERNEL_RESERVED_LIMIT);
}

static int memory_init_impl(multiboot_info_t *info)
{
	uint32_t index;
	multiboot_map_entry_t *entry;
	uint32_t mmap_end;

	if (!info || !info->mmap_addr || !info->mmap_length) {
		return -1;
	}

	memory_state.boot_info = info;
	memory_state.total_pages = 0;
	memory_state.free_pages = 0;
	ft_memset(memory_state.bitmap, 0, sizeof(memory_state.bitmap));
	ft_memset(memory_state.pages, 0, sizeof(memory_state.pages));

	memory_state.total_pages = compute_highest_address(info) /
		MEMORY_PAGE_SIZE;
	if (memory_state.total_pages > MEMORY_MAX_PAGES) {
		memory_state.total_pages = MEMORY_MAX_PAGES;
	}

	for (index = 0; index < memory_state.total_pages; index++) {
		page_init(index);
	}

	entry = (multiboot_map_entry_t *)info->mmap_addr;
	mmap_end = info->mmap_addr + info->mmap_length;
	while ((uint32_t)entry < mmap_end) {
		if (entry->type == 1) {
			uint32_t start;
			uint32_t end;

			start = (uint32_t)entry->base_addr;
			end = (uint32_t)(entry->base_addr + entry->length);
			if (end > MEMORY_MAX_PHYSICAL_MEMORY) {
				end = MEMORY_MAX_PHYSICAL_MEMORY;
			}
			mark_range_free(start, end);
		}

		entry = (multiboot_map_entry_t *)((uint32_t)entry +
			entry->size + sizeof(entry->size));
	}

	reserve_bootloader_metadata(info);
	reserve_kernel_space();
	return 0;
}

static memory_page_t *memory_get_page_impl(uint32_t address)
{
	uint32_t index;

	if (memory_state.total_pages == 0) {
		return NULL;
	}

	address = align_down(address);
	index = address / MEMORY_PAGE_SIZE;
	if (index >= memory_state.total_pages) {
		return NULL;
	}

	return &memory_state.pages[index];
}

static memory_page_t *memory_create_page_impl(uint32_t address)
{
	memory_page_t *page;

	page = memory_get_page_impl(address);
	if (!page) {
		return NULL;
	}

	if (page->flags & MEMORY_PAGE_FLAG_RESERVED) {
		return NULL;
	}

	if (page->flags & MEMORY_PAGE_FLAG_FREE) {
		page_mark_allocated(page->index);
	}

	return page;
}

static memory_page_t *memory_allocate_page_impl(void)
{
	uint32_t index;

	if (memory_state.total_pages == 0) {
		return NULL;
	}

	for (index = 0; index < memory_state.total_pages; index++) {
		if (bitmap_test(index)) {
			continue;
		}

		page_mark_allocated(index);
		return &memory_state.pages[index];
	}

	return NULL;
}

static void memory_free_page_impl(uint32_t address)
{
	memory_page_t *page;

	page = memory_get_page_impl(address);
	if (!page) {
		return;
	}

	if (page->flags & MEMORY_PAGE_FLAG_RESERVED) {
		return;
	}

	page_mark_free(page->index);
}

static uint32_t memory_get_total_pages_impl(void)
{
	return memory_state.total_pages;
}

static uint32_t memory_get_free_pages_impl(void)
{
	return memory_state.free_pages;
}

static uint32_t memory_get_page_size_impl(void)
{
	return MEMORY_PAGE_SIZE;
}

int memory_init(multiboot_info_t *info)
{
	return memory_init_impl(info);
}

memory_page_t *memory_get_page(uint32_t address)
{
	return memory_get_page_impl(address);
}

memory_page_t *memory_create_page(uint32_t address)
{
	return memory_create_page_impl(address);
}

memory_page_t *memory_allocate_page(void)
{
	return memory_allocate_page_impl();
}

void memory_free_page(uint32_t address)
{
	memory_free_page_impl(address);
}

uint32_t memory_get_total_pages(void)
{
	return memory_get_total_pages_impl();
}

uint32_t memory_get_free_pages(void)
{
	return memory_get_free_pages_impl();
}

uint32_t memory_get_page_size(void)
{
	return memory_get_page_size_impl();
}

void memory_dump_state(void)
{
	uint32_t used_pages;
	uint32_t index;
	uint32_t preview_pages;
	char state;

	used_pages = memory_state.total_pages - memory_state.free_pages;
	preview_pages = memory_state.total_pages;
	if (preview_pages > 32U) {
		preview_pages = 32U;
	}

	printf("\n[MEMORY] page size: %u bytes\n", memory_get_page_size());
	printf("[MEMORY] total pages: %u\n", memory_state.total_pages);
	printf("[MEMORY] free pages: %u\n", memory_state.free_pages);
	printf("[MEMORY] used pages: %u\n", used_pages);
	printf("[MEMORY] preview: ");
	for (index = 0; index < preview_pages; index++) {
		state = 'R';
		if (memory_state.pages[index].flags & MEMORY_PAGE_FLAG_FREE) {
			state = 'F';
		} else if (!(memory_state.pages[index].flags &
				MEMORY_PAGE_FLAG_RESERVED)) {
			state = 'A';
		}
		printf("%c", state);
	}
	printf("\n");
}
