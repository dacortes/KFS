// SPDX-License-Identifier: GPL-2.0

#pragma once

#include <stddef.h>

/**
 * @brief Debug wrapper for vmalloc with logging
 */
void *debug_vmalloc(size_t size, const char *caller);

/**
 * @brief Debug wrapper for vfree with logging
 */
void debug_vfree(void *ptr, const char *caller);

/**
 * @brief Print memory allocation statistics
 */
void memory_debug_print_stats(void);
