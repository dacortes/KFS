// SPDX-License-Identifier: GPL-2.0

#include <paging.h>
#include <pmm.h>

#define PAGE_DIRECTORY_ENTRIES	1024
#define PAGE_TABLE_ENTRIES	1024
#define PAGE_FRAME_MASK		0xFFFFF000
#define IDENTITY_LIMIT		0x00400000
#define BOOTSTRAP_TABLE_COUNT	((HHDM_LIMIT / \
				(PAGE_TABLE_ENTRIES * PAGE_SIZE)) + 1)

static uint32_t page_directory[PAGE_DIRECTORY_ENTRIES]
	__aligned(PAGE_SIZE);
static uint32_t bootstrap_tables[BOOTSTRAP_TABLE_COUNT][PAGE_TABLE_ENTRIES]
	__aligned(PAGE_SIZE);
static uint32_t bootstrap_table_count;
static int paging_active;

static inline uint32_t page_directory_index(uint32_t addr)
{
	return (addr >> 22);
}

static inline uint32_t page_table_index(uint32_t addr)
{
	return ((addr >> 12) & 0x3FF);
}

static inline void paging_flush_page(uint32_t addr)
{
	if (paging_active)
		asm volatile ("invlpg (%0)" : : "r" (addr) : "memory");
}

static uint32_t *paging_alloc_table(void)
{
	uint32_t frame;
	uint32_t *table;

	if (bootstrap_table_count < BOOTSTRAP_TABLE_COUNT) {
		table = bootstrap_tables[bootstrap_table_count];
		bootstrap_table_count++;
		return table;
	}

	frame = pmm_alloc_frame();
	if (!frame)
		return NULL;
	table = (uint32_t *)phys_to_virt(frame);
	return table;
}

static uint32_t *paging_get_table(uint32_t virt_addr, int create)
{
	uint32_t pde_index;
	uint32_t pde;
	uint32_t *page_table;
	uint32_t frame;

	pde_index = page_directory_index(virt_addr);
	pde = page_directory[pde_index];
	if (pde & PAGE_PRESENT)
		return ((uint32_t *)phys_to_virt(pde & PAGE_FRAME_MASK));
	if (!create)
		return NULL;

	page_table = paging_alloc_table();
	if (!page_table)
		return NULL;

	frame = virt_to_phys(page_table);
	ft_memset(page_table, 0, PAGE_SIZE);
	page_directory[pde_index] = (frame & PAGE_FRAME_MASK)
		| PAGE_PRESENT | PAGE_WRITE;
	return page_table;
}

static int paging_map_single(uint32_t virt_addr, uint32_t phys_addr,
	uint32_t flags)
{
	uint32_t *page_table;
	uint32_t pte_index;

	page_table = paging_get_table(virt_addr, 1);
	if (!page_table)
		return -1;

	pte_index = page_table_index(virt_addr);
	page_table[pte_index] = (phys_addr & PAGE_FRAME_MASK)
		| (flags | PAGE_PRESENT);
	paging_flush_page(virt_addr);
	return 0;
}

static void paging_unmap_single(uint32_t virt_addr)
{
	uint32_t *page_table;

	page_table = paging_get_table(virt_addr, 0);
	if (!page_table)
		return;

	page_table[page_table_index(virt_addr)] = 0;
	paging_flush_page(virt_addr);
}

static void paging_enable(void)
{
	uint32_t cr0;
	uint32_t directory_phys;

	directory_phys = virt_to_phys(page_directory);
	asm volatile ("mov %0, %%cr3" : : "r" (directory_phys) : "memory");
	asm volatile ("mov %%cr0, %0" : "=r" (cr0));
	cr0 |= 0x80000000;
	asm volatile ("mov %0, %%cr0" : : "r" (cr0) : "memory");
	paging_active = 1;
}

int paging_init(uint32_t physical_limit)
{
	uint32_t addr;
	int status;

	if (physical_limit > HHDM_LIMIT)
		physical_limit = HHDM_LIMIT;
	if (physical_limit < IDENTITY_LIMIT)
		physical_limit = IDENTITY_LIMIT;

	ft_memset(page_directory, 0, sizeof(page_directory));
	ft_memset(bootstrap_tables, 0, sizeof(bootstrap_tables));
	bootstrap_table_count = 0;
	paging_active = 0;

	for (addr = 0; addr < physical_limit; addr += PAGE_SIZE) {
		status = paging_map_single(HHDM_BASE + addr, addr, PAGE_WRITE);
		if (status != 0) {
			printf("ERROR: paging HHDM failed at 0x%x\n", addr);
			return -1;
		}
	}

	for (addr = 0; addr < IDENTITY_LIMIT; addr += PAGE_SIZE) {
		status = paging_map_single(addr, addr, PAGE_WRITE);
		if (status != 0) {
			printf("ERROR: paging identity failed at 0x%x\n", addr);
			return -1;
		}
	}

	paging_enable();
	return 0;
}

int paging_map_page(uint32_t virt_addr, uint32_t phys_addr, uint32_t flags)
{
	return paging_map_single(virt_addr, phys_addr, flags);
}

int paging_map_range(uint32_t virt_addr, uint32_t phys_addr, size_t count,
	uint32_t flags)
{
	size_t index;

	for (index = 0; index < count; index++) {
		if (paging_map_single(virt_addr + (index * PAGE_SIZE),
			phys_addr + (index * PAGE_SIZE), flags) != 0)
			return -1;
	}

	return 0;
}

void paging_unmap_page(uint32_t virt_addr)
{
	paging_unmap_single(virt_addr);
}

void paging_unmap_range(uint32_t virt_addr, size_t count)
{
	size_t index;

	for (index = 0; index < count; index++)
		paging_unmap_single(virt_addr + (index * PAGE_SIZE));
}
