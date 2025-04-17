#ifndef PROMPT_H
#define PROMPT_H

#include <string>
#include <unistd.h>
#include <cstdlib>
#include <cstring>

#define COLOR_RESET "\033[0m"
#define COLOR_RED "\033[31m"
#define COLOR_GREEN "\033[32m"
#define COLOR_YELLOW "\033[33m"
#define COLOR_BLUE "\033[34m"
#define COLOR_MAGENTA "\033[35m"
#define COLOR_CYAN "\033[36m"
#define COLOR_WHITE "\033[37m"
#define BOLD "\033[1m"

// Modified get_prompt() function
std::string get_prompt(); // Ensure this matches the definition

# endif