// SPDX-License-Identifier: GPL-2.0

#pragma once

#include <system.h>
#include <helper.h>
#include <stdbool.h>

#ifndef MAX_WORD
#define MAX_WORD 256
#endif

// esto se debe cambiar una vez se tengan los archivos(entrys) programados
#ifndef NUM_COMMANDS
#define NUM_COMMANDS 5
#endif

#ifndef MAX_LINE
#define MAX_LINE 256
#endif

// cambiar el max_word por memoria dinamica una vez se haga
typedef struct s_token {
	char		word[MAX_WORD];
	uint16_t	type;
}	t_token;

typedef struct s_shell {
	uint32_t	lv;
	char		***history;
	char		line[MAX_LINE];
	t_token		tokens[MAX_WORD];
	char		commands[NUM_COMMANDS][MAX_WORD];
	uint16_t	(*create_tokens)(t_shell *self, char *line);
	uint16_t	(*clear)(t_shell *self);
	uint16_t	(*execute)(t_shell *self);
} t_shell;

void	shell_init(t_shell *self);