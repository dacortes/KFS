// SPDX-License-Identifier: GPL-2.0

#include <memory.h>
#include <pmm.h>

#define MEMORY_MAGIC_KERNEL	0x4b4d454d
#define MEMORY_MAGIC_VIRTUAL	0x564d454d

typedef struct memory_header {
	uint32_t magic;
	size_t size;
	size_t page_count;
} memory_header_t;

static size_t round_up_pages(size_t size)
{
	return (size + PAGE_SIZE - 1) / PAGE_SIZE;
}

static void *memory_alloc(size_t size, uint32_t magic)
{
	memory_header_t *header;
	size_t page_count;
	uint32_t phys_addr;

	if (!size)
		return NULL;

	page_count = round_up_pages(size + sizeof(memory_header_t));
	phys_addr = pmm_alloc_frame_range(page_count);
	if (!phys_addr)
		return NULL;

	header = (memory_header_t *)phys_addr;
	header->magic = magic;
	header->size = size;
	header->page_count = page_count;
	return (void *)(header + 1);
}

static void memory_free(void *ptr, uint32_t magic)
{
	memory_header_t *header;

	if (!ptr)
		return;

	header = ((memory_header_t *)ptr) - 1;
	if (header->magic != magic)
		return;

	header->magic = 0;
	pmm_free_frame_range((uint32_t)header, header->page_count);
}

static size_t memory_size(const void *ptr, uint32_t magic)
{
	const memory_header_t *header;

	if (!ptr)
		return 0;

	header = ((const memory_header_t *)ptr) - 1;
	if (header->magic != magic)
		return 0;

	return header->size;
}

void memory_init(void)
{
	/* PMM-backed allocators do not need additional bootstrap state. */
}

void *kmalloc(size_t size)
{
	return memory_alloc(size, MEMORY_MAGIC_KERNEL);
}

void kfree(void *ptr)
{
	memory_free(ptr, MEMORY_MAGIC_KERNEL);
}

size_t ksize(const void *ptr)
{
	return memory_size(ptr, MEMORY_MAGIC_KERNEL);
}

void *kbrk(size_t size)
{
	return kmalloc(size);
}

void *vmalloc(size_t size)
{
	return memory_alloc(size, MEMORY_MAGIC_VIRTUAL);
}

void vfree(void *ptr)
{
	memory_free(ptr, MEMORY_MAGIC_VIRTUAL);
}

size_t vsize(const void *ptr)
{
	return memory_size(ptr, MEMORY_MAGIC_VIRTUAL);
}

void *vbrk(size_t size)
{
	return vmalloc(size);
}