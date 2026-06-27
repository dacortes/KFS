// SPDX-License-Identifier: GPL-2.0

/**
 * @file test_builtins.cpp
 * @brief Unit tests for shell builtin commands
 */

#include <gtest/gtest.h>

#define write kfs_write
extern "C" {
	#include <kernel/memory/memory.h>
#include <builtins.h>
}
#undef write
#include <cstdarg>
#include <cstdio>
#include <cstring>

extern "C" {
extern int g_halt_system_calls;
extern int g_reboot_system_calls;
extern int g_switch_to_user_mode_calls;
extern int g_return_to_kernel_mode_calls;
extern int g_get_current_privilege_level_calls;
extern int g_current_privilege_level_value;
extern void (*g_last_user_mode_function)(void);
extern void *g_last_user_mode_stack_top;
extern int g_write_redirectable_calls;
void reset_builtin_stub_state(void);
int install_halt_jmp(void);
int install_reboot_jmp(void);
}

class BuiltinTest : public ::testing::Test {
protected:
	multiboot_map_entry_t mmap_entries[2];
	multiboot_info_t info;

	void SetUp() override
	{
		reset_builtin_stub_state();
		g_write_redirectable_calls = 0;
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

		info.mmap_addr = (uint32_t)mmap_entries;
		info.mmap_length = sizeof(mmap_entries);
		ASSERT_EQ(0, memory_init(&info));
	}
	void TearDown() override
	{
	}
};

static shell_t make_shell_with_words(const char *first,
					     const char *second)
{
	shell_t shell = {};

	shell.num_tk = second ? 3 : 2;
	strncpy(shell.token[1].word, first, MAX_WORD - 1);
	if (second)
		strncpy(shell.token[2].word, second, MAX_WORD - 1);

	return shell;
}

TEST_F(BuiltinTest, EchoPrintsSingleArgument)
{
	shell_t shell = make_shell_with_words("hello", NULL);

	EXPECT_EQ(cmd_echo(&shell), 0);
}

TEST_F(BuiltinTest, EchoPrintsMultipleArguments)
{
	shell_t shell = make_shell_with_words("hello", "world");

	EXPECT_EQ(cmd_echo(&shell), 0);
}


TEST_F(BuiltinTest, UserModeBuiltinSwitchesWhenInKernelMode)
{
	shell_t shell = {};

	g_current_privilege_level_value = 0;

	EXPECT_EQ(cmd_user_mode(&shell), 0);
	EXPECT_EQ(g_get_current_privilege_level_calls, 1);
	EXPECT_EQ(g_switch_to_user_mode_calls, 1);
	EXPECT_NE(g_last_user_mode_function, nullptr);
	EXPECT_NE(g_last_user_mode_stack_top, nullptr);
}

TEST_F(BuiltinTest, UserModeBuiltinSkipsSwitchWhenAlreadyInUserMode)
{
	shell_t shell = {};

	g_current_privilege_level_value = 3;

	EXPECT_EQ(cmd_user_mode(&shell), 0);
	EXPECT_EQ(g_get_current_privilege_level_calls, 1);
	EXPECT_EQ(g_switch_to_user_mode_calls, 0);
}

TEST_F(BuiltinTest, ShowModeReportsKernelMode)
{
	shell_t shell = {};

	g_current_privilege_level_value = 0;

	EXPECT_EQ(cmd_show_mode(&shell), 0);
}

TEST_F(BuiltinTest, ShowModeReportsUserMode)
{
	shell_t shell = {};

	g_current_privilege_level_value = 3;

	EXPECT_EQ(cmd_show_mode(&shell), 0);
}

TEST_F(BuiltinTest, ShowModeReportsUnknownPrivilege)
{
	shell_t shell = {};

	g_current_privilege_level_value = 1;

	EXPECT_EQ(cmd_show_mode(&shell), 0);
}

TEST_F(BuiltinTest, MemstatPrintsMemoryState)
{
	shell_t shell = {};

	EXPECT_EQ(cmd_memstat(&shell), 0);
}

TEST_F(BuiltinTest, MempageAllocatesAndFreesByIndex)
{
	shell_t shell = {};

	shell.num_tk = 3;
	strncpy(shell.token[1].word, "alloc", MAX_WORD - 1);
	strncpy(shell.token[2].word, "512", MAX_WORD - 1);
	EXPECT_EQ(cmd_mempage(&shell), 0);

	strncpy(shell.token[1].word, "show", MAX_WORD - 1);
	EXPECT_EQ(cmd_mempage(&shell), 0);

	strncpy(shell.token[1].word, "free", MAX_WORD - 1);
	EXPECT_EQ(cmd_mempage(&shell), 0);
}

TEST_F(BuiltinTest, MempageFallsBackToNextFreePage)
{
	shell_t shell = {};

	shell.num_tk = 3;
	strncpy(shell.token[1].word, "alloc", MAX_WORD - 1);
	strncpy(shell.token[2].word, "0", MAX_WORD - 1);
	EXPECT_EQ(cmd_mempage(&shell), 0);
}