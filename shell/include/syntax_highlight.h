#ifndef SYNTAX_HIGHTLIGHT_H
#define SYNTAX_HIGHTLIGHT_H

// Add these near the top of main.cpp
#include <iostream>
#include <vector>
#include <string>
#include <readline/readline.h>
#include <readline/history.h>
#include "lexer.h"
#include "auto_suggestion.h"
#include "prompt.h"

#define ERROR_COLOR "\001\033[1;31m\002"
#define RESET_COLOR "\001\033[0m\002"

// Add before process_line function
int check_syntax(const char *line);
extern "C" void redisplay_hook();
int is_quoted(char *text, int index);

#endif