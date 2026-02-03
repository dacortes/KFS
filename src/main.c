// SPDX-License-Identifier: GPL-2.0

/**
 * @file main.c
 * @brief KFS kernel entry point
 * 
 * Main entry point for the KFS kernel. Initializes the kernel
 * and starts execution.
 */

#include <math/math.h>

/**
 * Main entry point for the kernel.
 * 
 * Initializes the kernel and demonstrates basic functionality.
 * 
 * @return Does not return (infinite loop in real kernel)
 */
int main(void)
{
	int result = math_add(5, 3);
	(void)result;
}
