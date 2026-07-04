// SPDX-License-Identifier: GPL-2.0

#include <builtins.h>
#include <memory.h>
#include <pmm.h>

#define MEMORY_SLOTS 16

typedef struct memory_slot {
	void *ptr;
	size_t size;
} memory_slot_t;

static memory_slot_t kernel_slots[MEMORY_SLOTS];
static memory_slot_t virtual_slots[MEMORY_SLOTS];

static memory_slot_t *get_slot(memory_slot_t *slots, size_t count, int id)
{
	if (id < 0 || (size_t)id >= count)
		return NULL;
	return &slots[id];
}

static int store_allocation(memory_slot_t *slots, size_t count, void *ptr,
	size_t size)
{
	for (size_t i = 0; i < count; i++) {
		if (!slots[i].ptr) {
			slots[i].ptr = ptr;
			slots[i].size = size;
			return (int)i;
		}
	}

	return -1;
}

static void print_slots(const char *label, memory_slot_t *slots, size_t count)
{
	printf("%s allocations:\n", label);
	for (size_t i = 0; i < count; i++) {
		if (!slots[i].ptr)
			continue;
		printf("  [%u] ptr=0x%x size=%u\n", (uint32_t)i,
			(uint32_t)slots[i].ptr, (uint32_t)slots[i].size);
	}
}

static void print_usage(void)
{
	printf("memory stats\n");
	printf("memory test\n");
	printf("memory k alloc <bytes>\n");
	printf("memory k free <id>\n");
	printf("memory v alloc <bytes>\n");
	printf("memory v free <id>\n");
}

static int handle_alloc(shell_t *self, memory_slot_t *slots, size_t count,
	int is_virtual)
{
	void *ptr;
	size_t size;
	int id;

	if (self->num_tk < 4) {
		print_usage();
		return -1;
	}

	size = (size_t)ft_atoi(self->token[3].word);
	if (!size) {
		printf("Invalid size: %s\n", self->token[3].word);
		return -1;
	}

	ptr = is_virtual ? vmalloc(size) : kmalloc(size);
	if (!ptr) {
		printf("Allocation failed\n");
		return -1;
	}

	id = store_allocation(slots, count, ptr, size);
	if (id < 0) {
		printf("No free memory slots available\n");
		if (is_virtual)
			vfree(ptr);
		else
			kfree(ptr);
		return -1;
	}

	printf("%s alloc[%d] ptr=0x%x size=%u\n",
		is_virtual ? "virtual" : "kernel", id,
		(uint32_t)ptr, (uint32_t)size);
	return 0;
}

static int handle_free(shell_t *self, memory_slot_t *slots, size_t count,
	int is_virtual)
{
	memory_slot_t *slot;
	int id;

	if (self->num_tk < 4) {
		print_usage();
		return -1;
	}

	id = ft_atoi(self->token[3].word);
	slot = get_slot(slots, count, id);
	if (!slot || !slot->ptr) {
		printf("Invalid slot: %d\n", id);
		return -1;
	}

	if (is_virtual)
		vfree(slot->ptr);
	else
		kfree(slot->ptr);

	printf("%s free[%d] ptr=0x%x size=%u\n",
		is_virtual ? "virtual" : "kernel", id,
		(uint32_t)slot->ptr, (uint32_t)slot->size);
	slot->ptr = NULL;
	slot->size = 0;
	return 0;
}

static int do_test(void)
{
	void *kptr = kmalloc(64);
	void *vptr = vmalloc(64);

	printf("memory test\n");
	printf("  kernel ptr: 0x%x size=%u\n", (uint32_t)kptr,
		(uint32_t)ksize(kptr));
	printf("  virtual ptr: 0x%x size=%u\n", (uint32_t)vptr,
		(uint32_t)vsize(vptr));
	if (kptr)
		kfree(kptr);
	if (vptr)
		vfree(vptr);
	printf("memory test complete\n");
	return 0;
}

int cmd_memory(shell_t *self)
{
	if (self->num_tk < 2 || !ft_strcmp(self->token[1].word, "help")) {
		print_usage();
		return 0;
	}

	if (!ft_strcmp(self->token[1].word, "stats")) {
		printf("PMM free pages: %u\n", (uint32_t)pmm_get_free_frame_count());
		printf("PMM used pages: %u\n", (uint32_t)pmm_get_used_frame_count());
		print_slots("kernel", kernel_slots, MEMORY_SLOTS);
		print_slots("virtual", virtual_slots, MEMORY_SLOTS);
		return 0;
	}

	if (!ft_strcmp(self->token[1].word, "test"))
		return do_test();

	if (self->num_tk < 3) {
		print_usage();
		return -1;
	}

	if (!ft_strcmp(self->token[1].word, "k")) {
		if (!ft_strcmp(self->token[2].word, "alloc"))
			return handle_alloc(self, kernel_slots, MEMORY_SLOTS, 0);
		if (!ft_strcmp(self->token[2].word, "free"))
			return handle_free(self, kernel_slots, MEMORY_SLOTS, 0);
	}

	if (!ft_strcmp(self->token[1].word, "v")) {
		if (!ft_strcmp(self->token[2].word, "alloc"))
			return handle_alloc(self, virtual_slots, MEMORY_SLOTS, 1);
		if (!ft_strcmp(self->token[2].word, "free"))
			return handle_free(self, virtual_slots, MEMORY_SLOTS, 1);
	}

	print_usage();
	return -1;
}