// SPDX-License-Identifier: GPL-2.0

/**
 * @file shell_debug.c
 * @brief Debugging utilities for shell memory management
 */

#include <shell.h>
#include <print.h>

void shell_debug_print_structure(const shell_t *self)
{
	if (!self) {
		printf("ERROR: shell pointer is NULL\n");
		return;
	}

	printf("\n=== Shell Structure Debug ===\n");
	printf("  num_tk:    %u\n", self->num_tk);
	printf("  capacity:  %u\n", self->capacity);
	printf("  tokens:    0x%x\n", (uint32_t)self->tokens);
	printf("  line:      0x%x\n", (uint32_t)self->line);
	printf("  lv:        %u\n", self->lv);
	printf("============================\n");
}

void shell_debug_print_tokens(const shell_t *self)
{
	if (!self) {
		printf("ERROR: shell pointer is NULL\n");
		return;
	}

	if (!self->tokens || self->num_tk == 0) {
		printf("No tokens\n");
		return;
	}

	printf("\n=== Tokens Debug ===\n");
	printf("Total tokens: %u (capacity: %u)\n", self->num_tk, self->capacity);

	for (uint32_t i = 0; i < self->num_tk; i++) {
		const token_t *tk = &self->tokens[i];
		printf("  [%u] word=0x%x (%s) type=%u\n",
			i, (uint32_t)tk->word, tk->word ? tk->word : "(null)", tk->type);
	}
	printf("====================\n");
}

void shell_debug_validate(const shell_t *self)
{
	if (!self) {
		printf("ERROR: shell pointer is NULL\n");
		return;
	}

	printf("\n=== Shell Validation ===\n");

	/* Validate num_tk and capacity */
	if (self->num_tk > self->capacity) {
		printf("ERROR: num_tk (%u) > capacity (%u)\n", self->num_tk, self->capacity);
	}

	/* Validate tokens pointer */
	if (self->capacity > 0 && !self->tokens) {
		printf("ERROR: capacity > 0 but tokens is NULL\n");
	}

	if (self->capacity == 0 && self->tokens) {
		printf("ERROR: capacity == 0 but tokens is not NULL\n");
	}

	/* Validate each token */
	for (uint32_t i = 0; i < self->num_tk; i++) {
		const token_t *tk = &self->tokens[i];
		if (!tk->word) {
			printf("ERROR: token[%u] has NULL word\n", i);
		}
	}

	printf("Validation complete\n");
	printf("========================\n");
}
