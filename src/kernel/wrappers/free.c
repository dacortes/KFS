// SPDX-License-Identifier: GPL-2.0

#include <helper.h>

/**
 * @brief Free dynamically allocated memory.
 *
 * This function releases memory previously allocated by functions that use
 * the kernel's virtual memory allocator (vmalloc), such as ft_calloc or
 * ft_strndup. It acts as a safe wrapper around vfree by checking for NULL
 * and returning a boolean status.
 *
 * @param ptr Pointer to the memory block to free. May be NULL; if so,
 *            the function returns false without performing any operation.
 *
 * @return    true if the memory was successfully freed (or if ptr is NULL,
 *            it returns false). false if ptr is NULL or if vfree fails
 *            (though vfree typically does not report errors).
 *
 * @note      This function is intended to be used with memory allocated via
 *            kernel allocators that rely on vmalloc. For memory allocated
 *            with kmalloc, use the corresponding deallocator (kfree).
 *
 * @warning   Calling this function on a pointer that was not allocated by
 *            vmalloc or on memory already freed results in undefined
 *            behaviour (likely a kernel panic). The caller must ensure that
 *            ptr is a valid allocated address or NULL.
 */

bool ft_free(void *ptr)
{
	if (!ptr)
		return false;

	vfree(ptr);
	return true;
}
