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

typedef struct s_shell t_shell;
typedef struct s_token t_token;

// cambiar el max_word por memoria dinamica una vez se haga
struct s_token {
	char		word[MAX_WORD];
	uint16_t	type;
};

struct s_shell {
	uint32_t	lv;
	char		***history;
	char		line[MAX_LINE];
	t_token		token[MAX_WORD];
	char		commands[NUM_COMMANDS][MAX_WORD];
	uint16_t	(*create_tokens)(t_shell *self, char *line);
	void		(*clear)(t_shell *self);
	uint16_t	(*execute)(t_shell *self);
};

void	shell_init(t_shell *self);