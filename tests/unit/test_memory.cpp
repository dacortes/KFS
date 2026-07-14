// SPDX-License-Identifier: GPL-2.0

/**
 * @file test_memory.cpp
 * @brief Unit tests for PMM-backed memory helpers and memory shell builtin
 */

#include <gtest/gtest.h>
#include <cstdint>
#include <cstring>

#define write kfs_write
extern "C" {
#include <builtins.h>
#include <pmm.h>
#include <memory.h>

void memory_init(void);
void *kmalloc(size_t size);
void kfree(void *ptr);
size_t ksize(const void *ptr);
void *kbrk(size_t size);
void *vmalloc(size_t size);
void vfree(void *ptr);
size_t vsize(const void *ptr);
void *vbrk(size_t size);
int memory_free_as(void *ptr, memory_space_t requester);
memory_space_t memory_owner(const void *ptr);
}
#undef write

#define TEST_POOL_PAGES 32

static uint8_t g_pool[TEST_POOL_PAGES * PAGE_SIZE] __attribute__((aligned(PAGE_SIZE)));
static size_t g_next_page;
static uint32_t g_last_free_addr;
static size_t g_last_free_count;
static size_t g_free_calls;

extern "C" {

uint32_t pmm_alloc_frame_range(size_t count)
{
	uint32_t phys_addr;

	if (!count || g_next_page + count > TEST_POOL_PAGES)
		return 0;

	phys_addr = (uint32_t)(uintptr_t)&g_pool[g_next_page * PAGE_SIZE];
	g_next_page += count;
	return phys_addr;
}

void pmm_free_frame_range(uint32_t phys_addr, size_t count)
{
	g_last_free_addr = phys_addr;
	g_last_free_count = count;
	g_free_calls++;
}

size_t pmm_get_free_frame_count(void)
{
	return TEST_POOL_PAGES - g_next_page;
}

size_t pmm_get_used_frame_count(void)
{
	return g_next_page;
}

} /* extern "C" */

class MemoryTest : public ::testing::Test {
protected:
	void SetUp() override
	{
		memset(g_pool, 0, sizeof(g_pool));
		g_next_page = 0;
		g_last_free_addr = 0;
		g_last_free_count = 0;
		g_free_calls = 0;
		memory_init();
	}
};

static shell_t make_memory_shell(const char *arg1, const char *arg2,
	const char *arg3, const char *arg4 = NULL)
{
	shell_t shell = {};

	strncpy(shell.token[0].word, "memory", MAX_WORD - 1);
	if (arg1)
		strncpy(shell.token[1].word, arg1, MAX_WORD - 1);
	if (arg2)
		strncpy(shell.token[2].word, arg2, MAX_WORD - 1);
	if (arg3)
		strncpy(shell.token[3].word, arg3, MAX_WORD - 1);
	if (arg4)
		strncpy(shell.token[4].word, arg4, MAX_WORD - 1);
	shell.num_tk = arg4 ? 5 : (arg3 ? 4 : (arg2 ? 3 : 2));
	return shell;
}

TEST_F(MemoryTest, KmallocKfreeAndKsizeWork)
{
	void *ptr = kmalloc(64);

	ASSERT_NE(ptr, nullptr);
	EXPECT_EQ(ksize(ptr), 64u);
	EXPECT_EQ(vsize(ptr), 0u);
	kfree(ptr);
	EXPECT_EQ(g_free_calls, 1u);
	EXPECT_EQ(g_last_free_count, 1u);
	EXPECT_EQ(ksize(ptr), 0u);
}

TEST_F(MemoryTest, VmallocVfreeAndVsizeWork)
{
	void *ptr = vmalloc(128);

	ASSERT_NE(ptr, nullptr);
	EXPECT_EQ(vsize(ptr), 128u);
	EXPECT_EQ(ksize(ptr), 0u);
	vfree(ptr);
	EXPECT_EQ(g_free_calls, 1u);
	EXPECT_EQ(g_last_free_count, 1u);
	EXPECT_EQ(vsize(ptr), 0u);
}

TEST_F(MemoryTest, DoubleFreeIsRejectedWithoutFreeingAgain)
{
	void *ptr = kmalloc(64);

	ASSERT_NE(ptr, nullptr);
	EXPECT_EQ(memory_free_as(ptr, MEMORY_SPACE_KERNEL), 0);
	EXPECT_EQ(g_free_calls, 1u);
	EXPECT_EQ(memory_free_as(ptr, MEMORY_SPACE_KERNEL), -1);
	EXPECT_EQ(g_free_calls, 1u);
}

TEST_F(MemoryTest, FreedAllocationsLoseTheirReportedSize)
{
	void *ptr = vmalloc(128);

	ASSERT_NE(ptr, nullptr);
	EXPECT_EQ(vsize(ptr), 128u);
	vfree(ptr);
	EXPECT_EQ(vsize(ptr), 0u);
	EXPECT_EQ(memory_owner(ptr), (memory_space_t)0);
}

TEST_F(MemoryTest, KbrkAndVbrkProxyToTheirAllocators)
{
	void *kernel_ptr = kbrk(32);
	void *virtual_ptr = vbrk(48);

	ASSERT_NE(kernel_ptr, nullptr);
	ASSERT_NE(virtual_ptr, nullptr);
	EXPECT_EQ(ksize(kernel_ptr), 32u);
	EXPECT_EQ(vsize(virtual_ptr), 48u);
}

TEST_F(MemoryTest, KernelAndVirtualAllocationsStaySeparate)
{
	void *kernel_ptr = kmalloc(16);
	void *virtual_ptr = vmalloc(16);

	ASSERT_NE(kernel_ptr, nullptr);
	ASSERT_NE(virtual_ptr, nullptr);
	EXPECT_EQ(memory_owner(kernel_ptr), MEMORY_SPACE_KERNEL);
	EXPECT_EQ(memory_owner(virtual_ptr), MEMORY_SPACE_USER);
	EXPECT_EQ(ksize(virtual_ptr), 0u);
	EXPECT_EQ(vsize(kernel_ptr), 0u);
}

TEST_F(MemoryTest, WrongSpaceFreeIsRejected)
{
	void *kernel_ptr = kmalloc(64);
	void *virtual_ptr = vmalloc(64);

	ASSERT_NE(kernel_ptr, nullptr);
	ASSERT_NE(virtual_ptr, nullptr);
	EXPECT_EQ(memory_free_as(kernel_ptr, MEMORY_SPACE_USER), -1);
	EXPECT_EQ(memory_free_as(virtual_ptr, MEMORY_SPACE_KERNEL), -1);
	EXPECT_EQ(g_free_calls, 0u);
	kfree(kernel_ptr);
	vfree(virtual_ptr);
}

TEST_F(MemoryTest, MemoryBuiltinAllocFreeAndTestCommands)
{
	shell_t shell = make_memory_shell("k", "alloc", "64");

	EXPECT_EQ(cmd_memory(&shell), 0);
	EXPECT_NE(g_next_page, 0u);

	shell = make_memory_shell("v", "alloc", "96");
	EXPECT_EQ(cmd_memory(&shell), 0);

	shell = make_memory_shell("k", "freeas", "v", "0");
	EXPECT_EQ(cmd_memory(&shell), -1);

	shell = make_memory_shell("k", "free", "0");
	EXPECT_EQ(cmd_memory(&shell), 0);

	shell = make_memory_shell("test", NULL, NULL);
	EXPECT_EQ(cmd_memory(&shell), 0);
}