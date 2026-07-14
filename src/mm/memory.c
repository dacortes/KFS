// SPDX-License-Identifier: GPL-2.0

#include <memory.h>
#include <pmm.h>
#include <print.h>

#define MEMORY_MAGIC_KERNEL	0x4b4d454d
#define MEMORY_MAGIC_VIRTUAL	0x564d454d
#define MEMORY_MAGIC_FREED	0x46524545

static size_t round_up_pages(size_t size)
{
	return (size + PAGE_SIZE - 1) / PAGE_SIZE;
}

static void *memory_alloc(size_t size, uint32_t magic,
	memory_space_t owner)
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
	header->owner = (uint8_t)owner;
	header->reserved[0] = 0;
	header->reserved[1] = 0;
	header->reserved[2] = 0;
	header->size = size;
	header->page_count = page_count;
	return (void *)(header + 1);
}

static int memory_free_checked(void *ptr, uint32_t magic,
	memory_space_t requester)
{
	memory_header_t *header;

	if (!ptr) {
		printf("ERROR: attempt to free a NULL pointer\n");
		return -1;
	}

	header = ((memory_header_t *)ptr) - 1;
	if (header->magic == MEMORY_MAGIC_FREED) {
		printf("ERROR: double free detected at 0x%x\n", (uint32_t)ptr);
		return -1;
	}
	if (header->magic != magic) {
		printf("ERROR: invalid free at 0x%x (magic=0x%x)\n",
			(uint32_t)ptr, header->magic);
		return -1;
	}
	if (header->owner != (uint8_t)requester) {
		printf("ERROR: memory owner mismatch at 0x%x\n",
			(uint32_t)ptr);
		return -1;
	}

	header->magic = MEMORY_MAGIC_FREED;
	header->owner = 0;
	pmm_free_frame_range((uint32_t)header, header->page_count);
	return 0;
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
	return memory_alloc(size, MEMORY_MAGIC_KERNEL, MEMORY_SPACE_KERNEL);
}

void kfree(void *ptr)
{
	(void)memory_free_checked(ptr, MEMORY_MAGIC_KERNEL,
		MEMORY_SPACE_KERNEL);
}

int memory_free_as(void *ptr, memory_space_t requester)
{
	memory_header_t *header;

	if (!ptr) {
		printf("ERROR: attempt to free a NULL pointer\n");
		return -1;
	}

	header = ((memory_header_t *)ptr) - 1;
	if (header->magic == MEMORY_MAGIC_KERNEL)
		return memory_free_checked(ptr, MEMORY_MAGIC_KERNEL, requester);
	if (header->magic == MEMORY_MAGIC_VIRTUAL)
		return memory_free_checked(ptr, MEMORY_MAGIC_VIRTUAL, requester);
	if (header->magic == MEMORY_MAGIC_FREED)
		printf("ERROR: double free detected at 0x%x\n", (uint32_t)ptr);
	else
		printf("ERROR: invalid free at 0x%x (magic=0x%x)\n",
			(uint32_t)ptr, header->magic);
	return -1;
}

size_t ksize(const void *ptr)
{
	return memory_size(ptr, MEMORY_MAGIC_KERNEL);
}

memory_space_t memory_owner(const void *ptr)
{
	const memory_header_t *header;

	if (!ptr)
		return 0;

	header = ((const memory_header_t *)ptr) - 1;
	if (header->magic == MEMORY_MAGIC_KERNEL)
		return (memory_space_t)header->owner;
	if (header->magic == MEMORY_MAGIC_VIRTUAL)
		return (memory_space_t)header->owner;
	if (header->magic == MEMORY_MAGIC_FREED)
		return 0;
	return 0;
}

void *kbrk(size_t size)
{
	return kmalloc(size);
}

void *vmalloc(size_t size)
{
	return memory_alloc(size, MEMORY_MAGIC_VIRTUAL, MEMORY_SPACE_USER);
}

void vfree(void *ptr)
{
	(void)memory_free_checked(ptr, MEMORY_MAGIC_VIRTUAL,
		MEMORY_SPACE_USER);
}

size_t vsize(const void *ptr)
{
	return memory_size(ptr, MEMORY_MAGIC_VIRTUAL);
}

void *vbrk(size_t size)
{
	return vmalloc(size);
}
