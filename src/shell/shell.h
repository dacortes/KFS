// SPDX-License-Identifier: GPL-2.0

#pragma once

#ifndef MAX_WORD
#define MAX_WORD 256
#endif

// esto se debe cambiar una vez se tengan los archivos(entrys) programados
#ifndef NUM_COMMANDS
#define NUM_COMMANDS 5
#endif

// cambiar el max_word por memoria dinamica una vez se haga
typedef struct s_token {
	char		word[MAX_WORD];
	uint16_t	type;
}	t_token;

typedef struct s_shell {
	uint32_t	lv;
	char		***history;
	char		*line;
	t_token		tokens[MAX_WORD];
	char		commands[NUM_COMMANDS];
	uint16_t	(*create_tokens)(char *line);
	uint16_t	(*shell_clear)(void);
} t_shell;