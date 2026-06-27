// SPDX-License-Identifier: GPL-2.0

#pragma once

#include <stdint.h>
#include <multiboot.h>
#include <print.h>
#include <stdbool.h>
#include <helper.h>

#define PAGE_SIZE 4096
#define PAGE_SHIFT 12

void pmm_init(multiboot_info_t *info);
uint32_t pmm_alloc_frame(void);
void pmm_free_frame(uint32_t phys_addr);
void pmm_free_frame_range(uint32_t phys_addr, size_t count);

size_t pmm_get_free_frame_count(void);
size_t pmm_get_total_frame_count(void);
size_t pmm_get_used_frame_count(void);
void pmm_print_stats(void);

void pmm_print_bitmap(void);
void pmm_print_memory_map(multiboot_info_t *info);
