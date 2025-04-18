#include "prompt.h"

std::string get_prompt()
{
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) == nullptr)
    {
        perror("getcwd() error");
        return COLOR_RED "alpShell> " COLOR_RESET;
    }

    const char *home = getenv("HOME");
    std::string current_dir = cwd;

    if (home)
    {
        size_t home_len = strlen(home);
        if (current_dir.substr(0, home_len) == home)
        {
            current_dir.replace(0, home_len, "~");
        }
    }

    // Directory in cyan, prompt symbol in green with bold
    // return COLOR_CYAN + current_dir + COLOR_RESET " " COLOR_GREEN BOLD "$ " COLOR_RESET;
    return std::string(COLOR_CYAN) + current_dir + COLOR_RESET + " " + COLOR_GREEN + BOLD + "$ " + COLOR_RESET + " ";
}