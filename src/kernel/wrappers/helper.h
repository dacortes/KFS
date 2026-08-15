// SPDX-License-Identifier: GPL-2.0

/**
 * @file helper.h
 * @brief String and memory helper functions for kernel use.
 *
 * Provides basic string manipulation and memory operations,
 * implemented either in x86 assembly or in C, for use in a
 * freestanding kernel environment without the standard C library.
 */

#pragma once

#include <../inc/stdint/stdint.h>
#include <memory.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ===================== String length ===================== */

/**
 * @brief Calculate the length of a null‑terminated string.
 *
 * @param s Pointer to the null‑terminated string. May be NULL.
 *
 * @return Number of characters before the null terminator,
 *         or 0 if @p s is NULL.
 */
unsigned int ft_strlen(const char *s);

/* ===================== String comparison ===================== */

/**
 * @brief Compare two null‑terminated strings lexicographically.
 *
 * Compares strings byte‑by‑byte using unsigned char values.
 *
 * @param s1 Pointer to the first string.
 * @param s2 Pointer to the second string.
 *
 * @return Negative value if @p s1 < @p s2, zero if equal,
 *         positive value if @p s1 > @p s2.
 */
int ft_strcmp(const char *s1, const char *s2);

/* ===================== String copy (unbounded) ===================== */

/**
 * @brief Copy a null‑terminated string from @p src to @p dst.
 *
 * Copies the string pointed to by @p src, including the null terminator,
 * to the buffer pointed to by @p dst. The caller must ensure @p dst has
 * enough space to hold the entire string plus its null terminator.
 *
 * @param dst Destination buffer.
 * @param src Source string.
 *
 * @return Pointer to @p dst, or NULL if either argument is NULL.
 */
char *ft_strcpy(char *dst, const char *src);

/* ===================== String copy with length limit ===================== */

/**
 * @brief Copy at most @p len bytes from @p src to @p dst, null‑padding.
 *
 * Copies up to @p len characters from @p src to @p dst. If @p src is shorter
 * than @p len, @p dst is padded with null bytes up to @p len. If @p src is
 * longer than @p len, only @p len bytes are copied (no null terminator added).
 *
 * @param dst Destination buffer (must have space for at least @p len bytes).
 * @param src Source string.
 * @param len Maximum number of bytes to copy.
 *
 * @return Pointer to @p dst, or an empty string (static) if either argument
 *         is NULL.
 */
char *ft_strncpy(char *dst, const char *src, unsigned int len);

/* ===================== Safe string copy with size limit ===================== */

/**
 * @brief Copy string with size limit, guaranteeing null termination.
 *
 * Copies up to @p dstsize - 1 characters from @p src to @p dst, always
 * null‑terminating the result. The returned value is the total length of
 * @p src, which can be used to detect truncation.
 *
 * @param dst     Destination buffer.
 * @param src     Source string.
 * @param dstsize Size of the destination buffer (including space for null).
 *
 * @return Total length of @p src (i.e., what would have been copied if
 *         @p dstsize were unlimited).
 */
size_t ft_strlcpy(char *dst, const char *src, size_t dstsize);

/* ===================== Memory operations ===================== */

/**
 * @brief Fill memory with a constant byte.
 *
 * Fills the first @p len bytes of the memory area pointed to by @p b with
 * the byte value @p c (converted to unsigned char).
 *
 * @param b   Pointer to the memory area to fill.
 * @param c   Value to fill (converted to unsigned char).
 * @param len Number of bytes to fill.
 *
 * @return Pointer to @p b.
 */
void *ft_memset(void *b, int c, unsigned int len);

/**
 * @brief Scan memory for a character.
 *
 * Scans the first @p n bytes of the memory area pointed to by @p s for
 * the byte @p c (converted to unsigned char).
 *
 * @param s Pointer to the memory area to scan.
 * @param c Byte to search for.
 * @param n Number of bytes to scan.
 *
 * @return Pointer to the matching byte, or NULL if not found.
 */
void *ft_memchr(const void *s, int c, unsigned int n);

/**
 * @brief Copy a block of memory from source to destination.
 *
 * Copies @p n bytes from @p src to @p dst. The memory areas must not overlap.
 * If they do, the behaviour is undefined.
 *
 * @param dst Destination memory area. May be NULL (then returns NULL).
 * @param src Source memory area. May be NULL (then returns NULL).
 * @param n   Number of bytes to copy.
 *
 * @return Pointer to @p dst, or NULL if either @p dst or @p src is NULL.
 *
 * @note This implementation does not handle overlapping regions.
 *       For overlapping areas, use ft_memmove instead.
 */
void *ft_memcpy(void *dst, const void *src, size_t n);

/* ===================== Character classification ===================== */

/**
 * @brief Check if a character is a decimal digit (0‑9).
 *
 * @param c Character to test (converted to unsigned char internally).
 *
 * @return Non‑zero if @p c is a digit, zero otherwise.
 */
int ft_isdigit(int c);

/**
 * @brief Check if a character is a blank (space or tab).
 *
 * @param c Character to test (converted to unsigned char internally).
 *
 * @return Non‑zero if @p c is a blank character (space or '\t'), zero otherwise.
 */
uint16_t ft_isblank(char c);

/* ===================== String to integer ===================== */

/**
 * @brief Convert a string to an integer.
 *
 * Ignores leading whitespace (as determined by ft_isblank) and handles
 * an optional plus or minus sign. Stops conversion at the first non‑digit
 * character.
 *
 * @param str Pointer to the null‑terminated string to convert.
 *
 * @return The converted integer value. On overflow or underflow, returns 0.
 */
int ft_atoi(const char *str);

/* ===================== Character search in string ===================== */

/**
 * @brief Locate a character in a string.
 *
 * Returns a pointer to the first occurrence of @p c in @p s. The null
 * terminator is considered part of the string and will be matched if @p c
 * is '\0'.
 *
 * @param s Pointer to the null‑terminated string to search.
 * @param c Character to locate (converted to unsigned char).
 *
 * @return Pointer to the first occurrence of @p c, or NULL if not found.
 */
char *ft_strchr(const char *s, int c);

/* ===================== String duplication ===================== */

/**
 * @brief Duplicate a string (up to @p n characters).
 *
 * Allocates memory using ft_calloc and copies at most @p n characters from
 * @p src to the new buffer. The new string is always null‑terminated.
 *
 * @param src Source string. May be NULL (then returns NULL).
 * @param n   Maximum number of characters to copy (excluding the null terminator,
 *            but the allocated buffer will include space for it).
 *
 * @return Pointer to the newly allocated string, or NULL if allocation fails
 *         or if @p src is NULL.
 *
 * @note The caller is responsible for freeing the returned memory with vfree.
 * @warning If @p src is shorter than @p n, the duplication stops at the
 *          null terminator, but the buffer is still sized for @p n+1.
 */
char *ft_strndup(const char *src, size_t n);

/* ===================== Memory allocation ===================== */

/**
 * @brief Allocate zero‑initialized memory for an array of elements.
 *
 * Computes total size as @p count * @p size, allocates the memory using
 * vmalloc, and fills it with zero bytes using ft_memset.
 *
 * @param count Number of elements to allocate.
 * @param size  Size in bytes of each element.
 *
 * @return Pointer to the allocated memory, or NULL on failure.
 *
 * @note The current implementation does not check for overflow in the
 *       multiplication. The caller should ensure that @p count * @p size
 *       does not exceed SIZE_MAX.
 * @warning Uses vmalloc – suitable for large/non‑contiguous allocations.
 *          For performance‑critical paths where physical contiguity is
 *          required, consider using kmalloc instead.
 */
void *ft_calloc(size_t count, size_t size);

/* ===================== Internal assembly helpers ===================== */

/*
 * The following functions are implemented in assembly (.s) and provide
 * the low‑level implementations for the corresponding wrapper functions.
 * They are not intended to be called directly; use the wrapper functions
 * declared above.
 */
unsigned int ft_strlen_asm(const char *s);
int ft_strcmp_asm(const char *s1, const char *s2);
char *ft_strcpy_asm(char *dst, const char *src);

/**
 * free - Release memory allocated by kernel allocators.
 * Full documentation is in the implementation file.
 * @file free.c
 */
bool free(void *ptr);

/**
 * ft_realloc - Resize an allocated memory block.
 * Full documentation is in the implementation file.
 * @file ft_realloc.c
 */
void *ft_realloc(void *ptr, size_t old_size, size_t new_size);

#ifdef __cplusplus
}
#endif
