// SPDX-License-Identifier: GPL-2.0

#include <helper.h>
#include <print.h>

/**
* @brief Resize a previously allocated memory block.
*
* This function changes the size of the memory block pointed to by `ptr`
* to `new_size` bytes. The contents of the block are preserved up to the
* lesser of the new and old sizes. If `new_size` is larger than `old_size`,
* the added memory is not initialized (unlike ft_calloc).
*
* @param ptr      Pointer to the memory block to resize. May be NULL.
* @param old_size Size of the original block in bytes. Must be accurate
*                 (the allocator does not store size metadata).
* @param new_size Desired new size in bytes.
*
* @return         Pointer to the newly allocated block. On success, the
*                 original `ptr` is invalidated (freed). On failure, NULL
*                 is returned and the original block remains untouched.
*                 If `new_size` is zero, NULL is returned and the original
*                 block is freed.
*
* @note           If `ptr` is NULL, the function behaves like
*                 `ft_calloc(1, new_size)` (allocates and zeroes).
*                 If `new_size` is 0, the function behaves like `free(ptr)`
*                 and returns NULL.
*
* @warning        The new memory area (if larger) is **not** zero-initialized.
*                 The caller must ensure that `old_size` is exactly the size
*                 originally allocated; passing an incorrect value leads to
*                 undefined behaviour (garbage data, buffer overflows, or
*                 memory corruption). This function uses vmalloc, so the
*                 new block may not be physically contiguous.
*/
void *ft_realloc(void *ptr, size_t old_size, size_t new_size)
{
	void *new_ptr;
	size_t copy_size;
	size_t actual_size;

	if (new_size == 0) {
		if (ptr)
			vfree(ptr);
		return NULL;
	}

	if (!ptr) {
		if (old_size != 0) {
			printf("WARNING: ft_realloc called with ptr=NULL but old_size=%u (ignored)\n",
				(uint32_t)old_size);
		}
		return ft_calloc(1, new_size);
	}

	/* Get actual size from allocator instead of trusting old_size */
	actual_size = vsize(ptr);
	if (actual_size == 0) {
		printf("ERROR: ft_realloc on invalid pointer 0x%x\n", (uint32_t)ptr);
		return NULL;
	}

	/* Use actual size for copying (more robust than old_size parameter) */
	if (actual_size < new_size)
		copy_size = actual_size;
	else
		copy_size = new_size;

	new_ptr = vmalloc(new_size);
	if (!new_ptr) {
		printf("ERROR: ft_realloc vmalloc(%u) failed\n", (uint32_t)new_size);
		return NULL;
	}

	if (copy_size > 0)
		ft_memcpy(new_ptr, ptr, copy_size);

	vfree(ptr);
	return new_ptr;
}
