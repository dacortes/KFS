#pragma once
#include <system.h>
#include <print.h>
#include <helper.h>
#ifdef __cplusplus
extern "C" {
#endif

#ifndef ERROR_NULL_PROMPT
#define ERROR_NULL_PROMPT "\033[1;31m[ERROR]\033[m The prompt is a null pointer\n"
#endif

#ifndef MAX_LINE_LEN
#define MAX_LINE_LEN 256
#endif

void	set_prompt(const char *prompt);
char	*readline(void);

#ifdef __cplusplus
}
#endif

