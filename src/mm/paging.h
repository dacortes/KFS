// SPDX-License-Identifier: GPL-2.0

#pragma once

#include <stdint.h>
#include <helper.h>

#define PAGE_PRESENT	0x001
#define PAGE_WRITE	0x002
#define PAGE_USER	0x004
#define __aligned(x) __attribute__((aligned(x)))


int paging_init(uint32_t physical_limit);
int paging_map_page(uint32_t virt_addr, uint32_t phys_addr, uint32_t flags);
int paging_map_range(uint32_t virt_addr, uint32_t phys_addr, size_t count,
	uint32_t flags);
void paging_unmap_page(uint32_t virt_addr);
void paging_unmap_range(uint32_t virt_addr, size_t count);