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

typedef struct shell_s shell_t;
typedef struct token_s token_t;

// cambiar el max_word por memoria dinamica una vez se haga
struct token_s {
	char		word[MAX_WORD];
	uint16_t	type;
	void 		(*clear)(token_t *self);
};

struct shell_s {
	uint32_t	num_tk;
	uint32_t	lv;
	char		***history;
	char		line[MAX_LINE];
	token_t		token[MAX_WORD];
	char		commands[NUM_COMMANDS][MAX_WORD];
	uint16_t	(*create_tokens)(shell_t *self, char *line);
	uint16_t	(*execute)(shell_t *self);
	void		(*print)(shell_t *self);
	void		(*clear)(shell_t *self);
};

void	shell_init(shell_t *self);