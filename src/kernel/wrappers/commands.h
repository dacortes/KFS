// SPDX-License-Identifier: GPL-2.0

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Stops the processor (HLT instruction)
 */
void	halt_system(void);

/**
* @brief Restart the system using the keyboard driver
*/
void	reboot_system(void);

/**
 * @brief Switches the processor to user mode and starts executing the given function
 * @param function The function to execute in user mode
 * @param stack_top The top of the stack to use in user mode
 */
void switch_to_user_mode(void (*function)(void), void *stack_top);

/**
 * @brief Returns the processor to kernel mode from user mode
 */
void return_to_kernel_mode(void);

/**
 * @brief Get the current privilege level (CPL) of the processor
 * @return The current privilege level (0-3)
 */
int get_current_privilege_level(void);


#ifdef __cplusplus
}
#endif