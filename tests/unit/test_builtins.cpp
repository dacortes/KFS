// SPDX-License-Identifier: GPL-2.0

/**
 * @file test_builtins.cpp
 * @brief Unit tests for shell builtin commands
 */

#include <gtest/gtest.h>

#define write kfs_write
extern "C" {
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
	void SetUp() override
	{
		reset_builtin_stub_state();
		g_write_redirectable_calls = 0;
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