// SPDX-License-Identifier: GPL-2.0
#include <builtins.h>
#include <shell.h>

static uint32_t word_size(const char *line)
{
	if (!line || !*line)
		return 0;

	uint32_t size = 0;
	while (line[size] && !ft_isblank(line[size]))
		size++;
	return size;
}

static void cut_blanks(uint32_t *i, const char *line)
{
	if (!line || !*line)
		return;

	while (line[*i] && ft_isblank(line[*i]))
		(*i)++;
}

void shell_clear_tokens(shell_t *self)
{
	if (!self)
		return;
	if (!self->tokens)
		return;

	for (uint32_t i = 0; i < self->num_tk; i++) {
		if (self->tokens[i].word) {
			free(self->tokens[i].word);
			self->tokens[i].word = NULL;
		}
	}
	free(self->tokens);
	self->tokens = NULL;
	self->num_tk = 0;
	self->capacity = 0;
}

void shell_clear(shell_t *self)
{
	if (!self)
		return;
	shell_clear_tokens(self);
	ft_memset(self->line, '\0', sizeof(self->line));
}

/**
* add_token - Append a new token to the dynamic array.
* @self: shell instance
* @word: pointer to start of word (not null-terminated)
* @len:  length of the word
*
* Returns: true on success, false on allocation failure.
*/
static bool add_token(shell_t *self, const char *word, uint32_t len)
{
	uint32_t new_cap;
	token_t *tk;
	token_t *new_tokens;

	if (!self || !word || len == 0)
		return false;

	/* Check if we need to expand capacity */
	if (self->num_tk >= self->capacity) {
		new_cap = (self->capacity == 0) ? 4 : self->capacity * 2;

		/* Prevent overflow: don't expand beyond reasonable limit */
		if (new_cap > 1024) {
			printf("ERROR: token capacity would exceed 1024 (current: %u)\n",
				new_cap);
			return false;
		}

		/* Allocate new token array */
		new_tokens = ft_realloc(
			self->tokens,
			self->capacity * sizeof(token_t),
			new_cap * sizeof(token_t)
		);

		if (!new_tokens) {
			printf("ERROR: ft_realloc failed for tokens (capacity %u -> %u)\n",
				self->capacity, new_cap);
			return false;
		}

		self->tokens = new_tokens;
		self->capacity = new_cap;
	}

	/* Validate that we have space and that tokens array is valid */
	if (!self->tokens || self->num_tk >= self->capacity) {
		printf("ERROR: token array invalid after realloc\n");
		return false;
	}

	/* Get pointer to next token slot */
	tk = &self->tokens[self->num_tk];

	/* Zero-initialize the token structure */
	tk->word = NULL;
	tk->type = 0;

	/* Duplicate the word string */
	tk->word = ft_strndup(word, len);
	if (!tk->word) {
		printf("ERROR: ft_strndup failed for token word (len=%u)\n", len);
		return false;
	}

	/* Increment token count */
	self->num_tk++;
	return true;
}

uint16_t create_tokens(shell_t *self, char *line)
{
	uint32_t i = 0;

	if (!line || !*line)
		return false;

	shell_clear_tokens(self);

	while (line[i]) {
		uint32_t size;
		cut_blanks(&i, line);
		if (!line[i])
			break;

		size = word_size(&line[i]);

		if (!add_token(self, &line[i], size)) {
			printf("ERROR: add_token failed at token #%u\n", self->num_tk);
			shell_clear_tokens(self);
			return 0;
		}

		/* Debug: warn if we're at a critical expansion point */
		if (self->num_tk == 8 || self->num_tk == 16) {
			printf("[DEBUG] Reached %u tokens (capacity: %u)\n",
				self->num_tk, self->capacity);
		}

		i += size;
	}
	return (self->num_tk > 0);
}

void printokens(const shell_t *self)
{
	if (!self->tokens || self->num_tk == 0)
		return;

	for (uint32_t i = 0; i < self->num_tk; i++) {
		if (i == 0)
			printf("\n[%sCOMMAND%s] %s\n", BLUE, END, self->tokens[i].word);
		else
			printf("  [%u] arg: %s\n", i, self->tokens[i].word);
	}
}

void token_clear(token_t *self)
{
	if (!self)
		return;
	if (self->word) {
		free(self->word);
		self->word = NULL;
	}
	self->type = 0;
}



static uint16_t execute(shell_t *self)
{
	char *cmd = self->tokens[0].word;
	size_t num = 0;

	while (self->builtins[num].name != NULL) {
		//printf("Comparing '%s' with '%s'\n", cmd, self->builtins[num].name);
		if (!ft_strcmp(cmd, self->builtins[num].name))
			return self->builtins[num].func(self);
		num++;
	}
	printf("%s[ERROR]%s: %s: command not found\n", RED, END, cmd);
	return 127;
}

// static uint16_t execute(shell_t *self)
// {
// 	char *cmd = self->token[0].word;

// 	for (size_t num = 0; num < NUM_COMMANDS; num++) {
// 		//printf("Comparing '%s' with '%s'\n", cmd, self->builtins[num].name);
// 		if (!ft_strcmp(cmd, self->builtins[num].name))
// 			return self->builtins[num].func(self);
// 	}
// 	printf("%s[ERROR]%s: %s: command not found\n", RED, END, cmd);
// 	return 127;
// }

void shell_init(shell_t *self, multiboot_info_t **info)
{
	uint32_t active = sys.active_terminal;
	terminal_t *term = &sys.terminals[active];
	static const builtin_t builtins[] = {
		{"reboot", cmd_reboot, "Reboot the system"},
		{"half",   cmd_half,   "Halt the CPU"},
		{"echo", cmd_echo, "Print arguments"},
		{"memory", cmd_memory, "Inspect and test memory helpers"},
		{"idt_probe", cmd_idt_probe, "Probe the IDT signal queue and handlers"},
		{"idt_fault", cmd_idt_fault, "Intentionally trigger a real divide-by-zero fault"},
		{"user_mode", cmd_user_mode, "Switch to user mode"},
		{"show_mode", cmd_show_mode, "Show current privilege level"},
		{"stack_kernel", cmd_info_stack_kernel, "Show stack kernel information"},
		{NULL, NULL, NULL}
	};

	self->tokens = NULL;
	self->num_tk = 0;
	self->capacity = 0;
	self->lv = 0;
	self->builtins = builtins;
	self->info = *info;
	self->history = (char ***)&term->history;
	ft_memset(self->line, '\0', sizeof(self->line));
	self->execute = execute;
	self->create_tokens = create_tokens;
	self->clear = shell_clear;
	self->print = printokens;
}
