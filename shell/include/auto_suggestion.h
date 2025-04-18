// Add these declarations to auto_suggestion.h
#pragma once

#include <string>
#include <vector>
#include <readline/readline.h>

// Ghost text color - dimmed gray
// Add RESET_COLOR if missing
#define GHOST_COLOR "\001\033[1;90m\002"
#define RESET_COLOR "\001\033[0m\002" // Ensure this is present

// Current suggestion state
extern std::string current_suggestion;
extern bool suggestion_active;

// Find a matching command from history based on current input
std::string findHistorySuggestion(const std::string &input);

// Display suggestion as ghost text
void displaySuggestion();

// Accept the current suggestion
int acceptSuggestion(int count, int key);

// Handle up/down keys with ghost text integration
int handleUpArrow(int count, int key);
int handleDownArrow(int count, int key);
