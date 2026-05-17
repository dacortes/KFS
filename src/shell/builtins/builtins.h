// SPDX-License-Identifier: GPL-2.0

#pragma once

#include <shell.h>
#include <print.h>

int cmd_echo(shell_t *self);
int cmd_reboot(shell_t *self);
int cmd_half(shell_t *self);
int cmd_user_mode(shell_t *self);
int cmd_show_mode(shell_t *self);