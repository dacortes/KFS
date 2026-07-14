// SPDX-License-Identifier: GPL-2.0

/**
 * @file test_readline_shell.cpp
 * @brief Unit tests for readline and shell tokenization/execute
 */

#include <gtest/gtest.h>
#include <cstring>

extern "C" {
#define write kfs_write
#include <ft_readline.h>
#include <shell.h>
#include <system.h>
#include <terminal.h>
#include <display.h>
#undef write
void reset_builtin_stub_state(void);
}

class ReadlineShellTest : public ::testing::Test {
protected:
    void SetUp() override
    {
        /* Initialize display and terminal for sys */
        static char video_memory[4000];
        memset(video_memory, 0, sizeof(video_memory));
        display_init(&sys.display);
        sys.display.videomemptr = video_memory;
        terminal_init(&sys.terminals[0], &sys.display, 0);
        sys.terminals[0].clear(&sys.terminals[0]);
        sys.active_terminal = 0;

        reset_builtin_stub_state();
    }
};

TEST_F(ReadlineShellTest, SetPromptUpdatesTerminalPrefix)
{
    const char *p = "TEST> ";
    set_prompt(p);
    EXPECT_STREQ(sys.terminals[sys.active_terminal].prefix, p);
}

TEST_F(ReadlineShellTest, ReadlineReturnsLineWhenReady)
{
    char out[256];
    terminal_t *term = &sys.terminals[0];
    /* simulate a typed line */
    strcpy(term->line, "hello world");
    term->line_ready = 1;

    char *res = readline(out);
    ASSERT_NE(res, nullptr);
    EXPECT_STREQ(out, "hello world");
    EXPECT_EQ(term->line_ready, 0);
}

TEST_F(ReadlineShellTest, CreateTokensAndExecuteBuiltin)
{
    shell_t shell;
    multiboot_info_t* dummy_info = {};

    shell_init(&shell,  &dummy_info);

    char line[] = "  echo  arg1 arg2";
    EXPECT_TRUE(shell.create_tokens(&shell, line));
    EXPECT_EQ(shell.num_tk, 3);

    /* execute echo builtin (should return 0) */
    int ret = shell.execute(&shell);
    EXPECT_EQ(ret, 0);
}

TEST_F(ReadlineShellTest, ExecuteUnknownCommandReturns127)
{
    shell_t shell;
    multiboot_info_t* dummy_info = {};

    shell_init(&shell,  &dummy_info);

    char line[] = "foobar";
    EXPECT_TRUE(shell.create_tokens(&shell, line));
    EXPECT_EQ(shell.num_tk, 1);

    int ret = shell.execute(&shell);
    EXPECT_EQ(ret, 127);
}
