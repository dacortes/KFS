// SPDX-License-Identifier: GPL-2.0

#pragma once

#include <stddef.h>

typedef enum memory_space {
	MEMORY_SPACE_KERNEL = 0x4b,
	MEMORY_SPACE_USER = 0x55,
} memory_space_t;

/**
 * @brief Initialize the kernel and virtual memory helper arenas.
 */
void memory_init(void);

/**
 * @brief Allocate bytes from the kernel heap helper.
 *
 * @param size Number of bytes to allocate.
 * @return Pointer to the allocated block, or NULL on failure.
 */
void *kmalloc(size_t size);

/**
 * @brief Free a block previously returned by kmalloc() or kbrk().
 *
 * @param ptr Pointer returned by the kernel heap helper.
 */
void kfree(void *ptr);

/**
 * @brief Free a block as if it was requested by a given space.
 *
 * @param ptr Pointer returned by kmalloc() or vmalloc().
 * @param requester Space requesting the free.
 * @return 0 on success, -1 if ownership does not match.
 */
int memory_free_as(void *ptr, memory_space_t requester);

/**
 * @brief Report the size of a kernel heap allocation.
 *
 * @param ptr Pointer returned by the kernel heap helper.
 * @return Allocation size in bytes, or 0 if ptr is invalid.
 */
size_t ksize(const void *ptr);

/**
 * @brief Return the ownership byte stored in the allocation header.
 *
 * @param ptr Pointer returned by kmalloc() or vmalloc().
 * @return MEMORY_SPACE_KERNEL, MEMORY_SPACE_USER, or 0 on invalid pointer.
 */
memory_space_t memory_owner(const void *ptr);

/**
 * @brief Allocate bytes from the kernel break helper.
 *
 * @param size Number of bytes to reserve.
 * @return Pointer to the reserved block, or NULL on failure.
 */
void *kbrk(size_t size);

/**
 * @brief Allocate bytes from the virtual memory helper.
 *
 * @param size Number of bytes to allocate.
 * @return Pointer to the allocated block, or NULL on failure.
 */
void *vmalloc(size_t size);

/**
 * @brief Free a block previously returned by vmalloc() or vbrk().
 *
 * @param ptr Pointer returned by the virtual memory helper.
 */
void vfree(void *ptr);

/**
 * @brief Report the size of a virtual memory allocation.
 *
 * @param ptr Pointer returned by the virtual memory helper.
 * @return Allocation size in bytes, or 0 if ptr is invalid.
 */
size_t vsize(const void *ptr);

/**
 * @brief Allocate bytes from the virtual break helper.
 *
 * @param size Number of bytes to reserve.
 * @return Pointer to the reserved block, or NULL on failure.
 */
void *vbrk(size_t size);