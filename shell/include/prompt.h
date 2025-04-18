#ifndef PROMPT_H
#define PROMPT_H

#include <string>
#include <unistd.h>
#include <cstdlib>
#include <cstring>

// #define COLOR_RESET "\033[0m"
// #define COLOR_RED "\033[31m"
// #define COLOR_GREEN "\033[32m"
// #define COLOR_YELLOW "\033[33m"
// #define COLOR_BLUE "\033[34m"
// #define COLOR_MAGENTA "\033[35m"
// #define COLOR_CYAN "\033[36m"
// #define COLOR_WHITE "\033[37m"
// #define BOLD "\033[1m"

// #define COLOR_RESET "\001\033[0m\002"
// #define COLOR_RED "\001\033[1;31m\002"
// #define COLOR_GREEN "\001\033[1;32m\002"
// #define COLOR_YELLOW "\001\033[1;33m\002"
// #define COLOR_BLUE "\001\033[1;34m\002"
// #define COLOR_CYAN "\001\033[1;36m\002"
// #define COLOR_MAGENTA "\001\033[1;35m\002"
// #define COLOR_WHITE "\001\033[1;37m\002"
// #define COLOR_RESET "\001\033[0m\002"
// #define BOLD "\001\033[1m\002"

// Add proper Readline escaping for color codes
#define COLOR_RESET "\001\033[0m\002"
#define COLOR_RED "\001\033[1;31m\002"
#define COLOR_GREEN "\001\033[1;32m\002"
#define COLOR_YELLOW "\001\033[1;33m\002"
#define COLOR_BLUE "\001\033[1;34m\002"
#define COLOR_CYAN "\001\033[1;36m\002"
#define COLOR_MAGENTA "\001\033[1;35m\002"
#define COLOR_WHITE "\001\033[1;37m\002"
#define COLOR_GREEN "\001\033[1;32m\002"
#define BOLD "\001\033[1m\002"

// Modified get_prompt() function
std::string get_prompt(); // Ensure this matches the definition

# endif