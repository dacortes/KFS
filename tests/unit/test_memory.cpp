// SPDX-License-Identifier: GPL-2.0

/**
 * @file test_memory.cpp
 * @brief Unit tests for the page frame allocator.
 */

#include <gtest/gtest.h>
#include <kernel/memory/memory.h>
#include <cstring>

class MemoryTest : public ::testing::Test {
protected:
	multiboot_map_entry_t mmap_entries[2];
	multiboot_info_t info;

	void SetUp() override
	{
		memset(&info, 0, sizeof(info));
		memset(mmap_entries, 0, sizeof(mmap_entries));

		mmap_entries[0].size = sizeof(multiboot_map_entry_t) -
			sizeof(uint32_t);
		mmap_entries[0].base_addr = 0x00100000U;
		mmap_entries[0].length = 0x00300000U;
		mmap_entries[0].type = 1;

		mmap_entries[1].size = sizeof(multiboot_map_entry_t) -
			sizeof(uint32_t);
		mmap_entries[1].base_addr = 0x00400000U;
		mmap_entries[1].length = 0x00400000U;
		mmap_entries[1].type = 2;

		info.mmap_addr = (uint32_t)(uintptr_t)mmap_entries;
		info.mmap_length = sizeof(mmap_entries);
		ASSERT_EQ(0, memory_init(&info));
	}
};

TEST_F(MemoryTest, InitializesPageCounts)
{
	EXPECT_EQ(2048U, memory_get_total_pages());
	EXPECT_EQ(512U, memory_get_free_pages());
	EXPECT_EQ(MEMORY_PAGE_SIZE, memory_get_page_size());
}

TEST_F(MemoryTest, GetsSamePageForAnyAddressInFrame)
{
	memory_page_t *page_a;
	memory_page_t *page_b;

	page_a = memory_get_page(0x00200000U);
	page_b = memory_get_page(0x00200ABCU);

	ASSERT_NE(nullptr, page_a);
	ASSERT_NE(nullptr, page_b);
	EXPECT_EQ(page_a, page_b);
	EXPECT_TRUE(page_a->flags & MEMORY_PAGE_FLAG_FREE);
}

TEST_F(MemoryTest, CreatesAndClaimsFreePage)
{
	memory_page_t *page;

	page = memory_create_page(0x00200000U);

	ASSERT_NE(nullptr, page);
	EXPECT_EQ(511U, memory_get_free_pages());
	EXPECT_EQ(0U, page->flags);
}

TEST_F(MemoryTest, AllocatesFirstFreePage)
{
	memory_page_t *page;

	page = memory_allocate_page();

	ASSERT_NE(nullptr, page);
	EXPECT_EQ(0x00200000U, page->address);
	EXPECT_EQ(511U, memory_get_free_pages());
	EXPECT_EQ(0U, page->flags);
}

TEST_F(MemoryTest, RejectsReservedPageCreation)
{
	memory_page_t *page;

	page = memory_create_page(0x00001000U);

	EXPECT_EQ(nullptr, page);
	EXPECT_EQ(512U, memory_get_free_pages());
}

TEST_F(MemoryTest, FreesAllocatedPage)
{
	memory_page_t *page;

	page = memory_create_page(0x00200000U);
	ASSERT_NE(nullptr, page);

	memory_free_page(0x00200000U);
	EXPECT_EQ(512U, memory_get_free_pages());
	EXPECT_TRUE(memory_get_page(0x00200000U)->flags & MEMORY_PAGE_FLAG_FREE);
}
