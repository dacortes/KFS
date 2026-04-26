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

#ifdef __cplusplus
}
#endif