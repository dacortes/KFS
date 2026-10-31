// SPDX-License-Identifier: GPL-2.0

#pragma once

#include <stdint.h>
#include <multiboot.h>
#include <print.h>
#include <stdbool.h>
#include <helper.h>

#define PAGE_SIZE 4096
#define PAGE_SHIFT 12

#define HHDM_BASE 0xC0000000
#define HHDM_LIMIT 0x02000000

static inline void *phys_to_virt(uint32_t phys_addr)
{
	return ((void *)(phys_addr + HHDM_BASE));
}

static inline uint32_t virt_to_phys(const void *virt_addr)
{
	return ((uint32_t)virt_addr - HHDM_BASE);
}

extern uint32_t endkernel;

void pmm_init(multiboot_info_t *info);
uint32_t pmm_alloc_frame(void);
uint32_t pmm_alloc_frame_range(size_t count);
void pmm_free_frame(uint32_t phys_addr);
void pmm_free_frame_range(uint32_t phys_addr, size_t count);

size_t pmm_get_free_frame_count(void);
size_t pmm_get_total_frame_count(void);
size_t pmm_get_used_frame_count(void);
void pmm_print_stats(void);

void pmm_print_bitmap(void);
void pmm_print_memory_map(multiboot_info_t *info);

