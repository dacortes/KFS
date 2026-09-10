// SPDX-License-Identifier: GPL-2.0

#include <helper.h>

/**
 * @brief Allocates and zero-initializes memory for an array of elements.
 *
 * This function computes the total required memory size by multiplying
 * @p count by @p size. It then allocates the requested memory region using
 * the kernel's virtual memory allocator (vmalloc). The allocated block is
 * subsequently filled with zero bytes using ft_memset to guarantee that
 * all bits are cleared before the pointer is returned to the caller.
 *
 * @param count Number of elements to allocate.
 * @param size  Size in bytes of each individual element.
 *
 * @return      On success, a pointer to the allocated and zeroed memory block.
 *              On failure (e.g., memory exhaustion), NULL is returned.
 *              If the computed total size is zero, the behavior depends
 *              on the underlying vmalloc implementation; typically, a valid
 *              non-NULL pointer may be returned, but this is not guaranteed.
 *
 * @note        The current implementation does not explicitly guard against
 *              integer overflow when calculating (count * size). The caller
 *              must ensure that the multiplication does not exceed SIZE_MAX
 *              to avoid undefined behavior. For safety-critical allocations,
 *              consider adding an overflow check prior to calling this
 *              function.
 *
 * @warning     This function uses vmalloc, which is appropriate for large
 *              or non‑contiguous physical allocations in the kernel address
 *              space. It is not intended for use in performance‑sensitive
 *              paths where physical contiguity is required (use kmalloc
 *              instead in such cases).
 */

 #include <print.h>

void		*ft_calloc(size_t count, size_t size)
{
	size_t	asig = size * count;
	void	*res = vmalloc(asig);

    if (!res) {
        printf("[T] ft_calloc: vmalloc(%u) FAILED\n", (unsigned)asig);
        return NULL;
    }
	ft_memset(res, 0, asig);
	return res;
}
