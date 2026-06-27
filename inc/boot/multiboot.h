#pragma once

#include <stdint.h>

typedef struct {
	uint32_t size;
	uint64_t base_addr;
	uint64_t length;
	uint32_t type;
} __attribute__((packed)) multiboot_map_entry_t;

typedef struct {
	uint32_t flags;
	uint32_t mem_lower;
	uint32_t mem_upper;
	uint32_t boot_device;
	uint32_t cmdline;
	uint32_t mods_count;
	uint32_t mods_addr;
	uint32_t syms[4];
	uint32_t mmap_length;
	uint32_t mmap_addr;
	uint32_t num;
	uint32_t size;
	uint32_t addr;
} __attribute__((packed)) multiboot_info_t;

#define MULTIBOOT_MAGIC 0x2BADB002

#define MULTIBOOT_INFO_MEMORY (1 << 0)
