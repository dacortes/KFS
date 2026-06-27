// SPDX-License-Identifier: GPL-2.0

#include <builtins.h>
#include <kernel/memory/memory.h>

static int parse_page_index(shell_t *self, uint32_t *index)
{
	int parsed;

	if (!self || !index || self->num_tk < 3) {
		return -1;
	}

	parsed = ft_atoi(self->token[2].word);
	if (parsed < 0) {
		return -1;
	}

	*index = (uint32_t)parsed;
	return 0;
}

static void print_page_state(memory_page_t *page)
{
	if (!page) {
		printf("[MEMPAGE] page not found\n");
		return;
	}

	printf("[MEMPAGE] index=%u address=0x%x ", page->index, page->address);
	if (page->flags & MEMORY_PAGE_FLAG_RESERVED) {
		printf("state=reserved\n");
	} else if (page->flags & MEMORY_PAGE_FLAG_FREE) {
		printf("state=free\n");
	} else {
		printf("state=allocated\n");
	}
}

static memory_page_t *allocate_page_from_index(uint32_t index)
{
	uint32_t total_pages;
	uint32_t current;
	uint32_t address;
	memory_page_t *page;

	total_pages = memory_get_total_pages();
	for (current = index; current < total_pages; current++) {
		address = current * memory_get_page_size();
		page = memory_create_page(address);
		if (page) {
			if (current != index) {
				printf("[MEMPAGE] page %u was unavailable, allocated page %u\n",
				       index, current);
			}
			return page;
		}
	}

	return NULL;
}

int cmd_mempage(shell_t *self)
{
	uint32_t index;
	uint32_t address;
	memory_page_t *page;

	if (!self || self->num_tk < 2) {
		printf("[MEMPAGE] usage: mempage <show|alloc|free> <index>\n");
		return -1;
	}

	if (ft_strcmp(self->token[1].word, "show") == 0) {
		if (parse_page_index(self, &index) < 0) {
			printf("[MEMPAGE] usage: mempage show <index>\n");
			return -1;
		}
		address = index * memory_get_page_size();
		print_page_state(memory_get_page(address));
		return 0;
	}

	if (parse_page_index(self, &index) < 0) {
		printf("[MEMPAGE] usage: mempage <show|alloc|free> <index>\n");
		return -1;
	}

	address = index * memory_get_page_size();
	if (ft_strcmp(self->token[1].word, "alloc") == 0) {
		page = memory_create_page(address);
		if (!page) {
			page = allocate_page_from_index(index);
		}
		if (!page) {
			printf("[MEMPAGE] cannot allocate page %u\n", index);
			return -1;
		}
		printf("[MEMPAGE] allocated page %u\n", page->index);
		print_page_state(page);
		return 0;
	}

	if (ft_strcmp(self->token[1].word, "free") == 0) {
		page = memory_get_page(address);
		if (!page || (page->flags & MEMORY_PAGE_FLAG_RESERVED)) {
			printf("[MEMPAGE] cannot free page %u\n", index);
			return -1;
		}
		memory_free_page(address);
		printf("[MEMPAGE] freed page %u\n", index);
		print_page_state(memory_get_page(address));
		return 0;
	}

	printf("[MEMPAGE] usage: mempage <show|alloc|free> <index>\n");
	return -1;
}