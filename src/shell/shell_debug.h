// SPDX-License-Identifier: GPL-2.0

#pragma once

#include <shell.h>

/**
 * @brief Print shell structure information for debugging
 */
void shell_debug_print_structure(const shell_t *self);

/**
 * @brief Print all tokens for debugging
 */
void shell_debug_print_tokens(const shell_t *self);

/**
 * @brief Validate shell structure integrity
 */
void shell_debug_validate(const shell_t *self);
