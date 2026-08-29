// SPDX-License-Identifier: GPL-2.0

/**
 * @file test_builtins.cpp
 * @brief Unit tests for shell builtin commands
 */

#include <gtest/gtest.h>

#include <system_log.h>

#define ft_free(ptr) kfree(ptr)


#define write kfs_write
extern "C" {
#include <builtins.h>
#include <stack_kernel.h>
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

static shell_t make_shell_with_words(const char* cmd, const char *first,
					     const char *second)
{
	shell_t shell = {};

	shell.tokens = (token_t *)malloc(sizeof(token_t) * 3);

	if (!shell.tokens) {
		// Handle allocation failure
		return shell;
	}

	shell.tokens[0].word = strndup(cmd, ft_strlen(cmd) + 1);
	if (!shell.tokens[0].word) {
		// Handle allocation failure
		return shell;
	}

	shell.num_tk = second ? 3 : 2;
	shell.tokens[1].word = strndup(first, ft_strlen(first) + 1);
	if (!shell.tokens[1].word) {
		// Handle allocation failure
		return shell;
	}
	if (second) {
		shell.tokens[2].word = strndup(second, ft_strlen(second) + 1);
		if (!shell.tokens[2].word) {
			// Handle allocation failure
			return shell;
		}
	}

	return shell;
}

TEST_F(BuiltinTest, EchoPrintsSingleArgument)
{
	shell_t shell = make_shell_with_words("echo", "hello", NULL);

	EXPECT_EQ(cmd_echo(&shell), 0);
}

TEST_F(BuiltinTest, EchoPrintsMultipleArguments)
{
	shell_t shell = make_shell_with_words("echo", "hello", "world");

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
TEST_F(BuiltinTest, IdtProbeReportsSignalQueueAndHandlers)
{
	shell_t shell = {};

	EXPECT_EQ(cmd_idt_probe(&shell), 0);
}
TEST_F(BuiltinTest, ShowModeReportsUnknownPrivilege)
{
	shell_t shell = {};

	g_current_privilege_level_value = 1;

	EXPECT_EQ(cmd_show_mode(&shell), 0);
}

TEST_F(BuiltinTest, HaltAndRebootBuiltinsInvokeSystemCalls)
{
	shell_t shell = {};

	EXPECT_EQ(cmd_half(&shell), 0);
	EXPECT_EQ(g_halt_system_calls, 1);

	reset_builtin_stub_state();
	EXPECT_EQ(cmd_reboot(&shell), 0);
	EXPECT_EQ(g_reboot_system_calls, 1);
}

TEST_F(BuiltinTest, StackKernelBuiltinPrintsMultibootInfo)
{
	shell_t shell = {};
	multiboot_info_t info = {};
	uint8_t buffer[64] = {0};
	multiboot_map_entry_t *entry = (multiboot_map_entry_t *)buffer;

	entry->size = sizeof(*entry);
	entry->base_addr = 0;
	entry->length = 1024 * 1024;
	entry->type = 1;
	info.mmap_addr = (uint32_t)(uintptr_t)buffer;
	info.mmap_length = sizeof(buffer);
	shell.info = &info;

	EXPECT_EQ(cmd_info_stack_kernel(&shell), 0);
}

TEST_F(BuiltinTest, SystemLogInitWriteDumpAndSetLogLevel)
{
	system_log_t log = {};
	terminal_t terminal = {};
	static char sink[256];

	terminal.write_string = [](terminal_t *self, const char *text) {
		(void)self;
		if (!text)
			return 0;
		strncpy(sink, text, sizeof(sink) - 1);
		sink[sizeof(sink) - 1] = '\0';
		return (int)strlen(text);
	};

	system_log_init(&log);
	log.write(&log, KERN_INFO, "hello");
	log.write(&log, KERN_DEBUG, "world");
	log.set_loglevel(&log, KERN_ERR, KERN_DEBUG);
	log.dump(&log, &terminal);

	EXPECT_EQ(log.entry_count, 2u);
	EXPECT_EQ(log.console_loglevel, KERN_ERR);
	EXPECT_EQ(log.syslog_loglevel, KERN_DEBUG);
	EXPECT_GT(strlen(sink), 0u);
}