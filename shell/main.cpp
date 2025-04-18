#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include <csignal>
#include <cstdlib> // For free(), getenv(), exit()
#include <cstdio>  // For perror()
#include <unistd.h> // For fork(), pid_t, chdir(), isatty()
#include <fstream> // For reading script files
#include <sys/stat.h>
#include <dirent.h>
#include <cstring>
#include <set>
// #include <sys/wait.h>
// #include <fcntl.h>

// --- Readline Headers ---

extern "C"
{
    #include <readline/readline.h>
    #include <readline/history.h>
}

// #include <readline/readline.h>
// #include <readline/history.h>
// -----------------------

#include "lexer.h"
#include "parser.h"
#include "command.h"
#include "executor.h"
#include "job_control.h"
#include "history.h"
#include "prompt.h"
#include "syntax_highlight.h"
#include "auto_suggestion.h"

static char** completion_callback(const char *text, int start, int end);
static char* command_generator(const char *text, int state);

static std::vector<std::string> path_dirs; // Parsed PATH directories
static std::set<std::string> command_matches; // Unique command matches found
static std::set<std::string>::iterator command_match_iter;

static char* command_generator(const char *text, int state) {
    // If state is 0, it's the first call for this completion attempt.
    // We need to find all possible command matches.
    for (const std::string &dir : path_dirs)
    {
        DIR *dirp = opendir(dir.c_str());
        if (!dirp)
        {
            continue;
        }

        struct dirent *dp;
        while ((dp = readdir(dirp)) != nullptr)
        {
            // ... existing match checking ...
        }
        closedir(dirp); // Moved INSIDE the if-block
    }
    if (state == 0) {
        command_matches.clear(); // Clear previous matches

        // 1. Get and parse PATH environment variable
        const char* path_env = getenv("PATH");
        if (!path_env) {
            path_env = "/bin:/usr/bin"; // Sensible default if PATH isn't set
        }
        std::string path_str(path_env);
        std::stringstream ss(path_str);
        std::string dir_name;
        path_dirs.clear();
        while (std::getline(ss, dir_name, ':')) {
            if (!dir_name.empty()) {
                path_dirs.push_back(dir_name);
            }
        }
        // Add current directory too? Some shells do, some don't implicitly. Let's skip for now.

        // 2. Iterate through PATH directories and find executable matches
        size_t text_len = strlen(text);
        struct stat st;

        for (const std::string& dir : path_dirs) {
            DIR *dirp = opendir(dir.c_str());
            if (!dirp) {
                continue; // Couldn't open directory, skip
            }

            struct dirent *dp;
            while ((dp = readdir(dirp)) != nullptr) {
                // Check if filename starts with the text we're completing
                if (strncmp(dp->d_name, text, text_len) == 0) {
                    // Construct full path
                    std::string full_path = dir + "/" + dp->d_name;

                    // Check if it's a regular file and executable
                    if (stat(full_path.c_str(), &st) == 0 && S_ISREG(st.st_mode) && (access(full_path.c_str(), X_OK) == 0))
                    {
                        command_matches.insert(dp->d_name); // Insert unique command name
                    }
                }
            }
            closedir(dirp);
        }
        // Initialize iterator for returning matches
        command_match_iter = command_matches.begin();
    }

    // Return next match (or NULL if no more matches)
    if (command_match_iter != command_matches.end()) {
        const std::string& match = *command_match_iter;
        ++command_match_iter;
        // Readline expects malloc'd strings, use strdup
        return strdup(match.c_str());
    }

    // No more matches
    return nullptr;
}


static char** completion_callback(const char *text, int start, int /*end*/) {
    // Don't do filename completion even if invoked on empty line.
    // rl_attempted_completion_over = 1; // Use this if you want to prevent further completion attempts

    // If 'start' is 0, we are completing the command itself (first word)
    if (start == 0) {
        // Use rl_completion_matches to call our generator and get results
        // It handles calling the generator with state 0, 1, 2... and collecting results
        return rl_completion_matches(text, command_generator);
    }
    // Otherwise (start != 0), assume filename completion
    else {
        // Use readline's default filename completer generator
        // We pass NULL as the generator to rl_completion_matches,
        // which tells it to use the default filename completer.
        // Alternatively, explicitly use rl_filename_completion_function:
        // return rl_completion_matches(text, rl_filename_completion_function);
         return rl_completion_matches(text, rl_filename_completion_function);
    }

    // Should not be reached
    // return nullptr;
}

static int update_suggestion_hook()
{
    current_suggestion = findHistorySuggestion(rl_line_buffer);
    suggestion_active = !current_suggestion.empty();
    return 0;
}

// Function to process a single line of command input
// Returns true to continue, false to exit
bool process_line(const std::string& line, bool is_interactive) {
    // if (line.empty()) {
    //     return true; // Ignore empty lines
    // }

    std::string trimmed_line = line;

    size_t first_char = trimmed_line.find_first_not_of(" \t");
    if (first_char == std::string::npos) {
        return true; // Line is empty or only whitespace
    }
    trimmed_line = trimmed_line.substr(first_char);

    if (trimmed_line.empty() || trimmed_line[0] == '#') {
        return true; // Skip empty lines and comments
    }



    // Only add interactive commands to history
    if (is_interactive) {
        // Add to readline's history if it came from readline
        // (We already did this before calling process_line for interactive mode)
        // Add to our internal history
        History::addHistory(line);
    }

    if (line == "ls")
    {
        system("ls --color=auto");
        return true;
    }

    // --- Built-in command checks ---
    if (line == "exit") {
        return false; // Signal loop to exit
    }

    // These built-ins might make less sense in non-interactive mode,
    // but we can leave them for now.
    if (is_interactive && line == "history") {
        History::printHistory();
        return true;
    }
    if (line == "jobs") { // jobs can be useful even in scripts sometimes
        JobControl::listJobs();
        return true;
    }

    // --- Tokenize and Parse ---
    Lexer lexer;
    std::vector<Token> tokens = lexer.tokenize(line);
    // Check for lexer errors (e.g., unterminated quote returns empty tokens)
    if (tokens.empty() && !line.empty()) { // Check !line.empty() to distinguish from truly empty input
        std::cerr << "alpShell: Error during tokenization." << std::endl;
        // Decide script behavior: continue or exit? Let's continue for now.
        return true; // Or return false if scripts should exit on lexer error
    }
    // Handle case where only END token is returned for empty valid input
    if (tokens.size() <= 1 && tokens[0].type == TokenType::END) {
         return true;
    }


    Parser parser;
    std::vector<Command> commands = parser.parse(tokens);

    if (commands.empty()) {
        // Parsing failed or resulted in nothing (e.g., comment lines if implemented)
        // Check if it was due to a parsing error message already printed by parser
        return true; // Continue processing script/interactive session
    }

    // --- Built-in command: cd ---
    if (commands[0].executable == "cd") {
        std::string target_dir;
        if (commands[0].arguments.empty()) {
            const char* home_dir = getenv("HOME");
            if (home_dir == nullptr) {
                std::cerr << "alpShell: cd: HOME not set" << std::endl;
                return true;
            }
            target_dir = home_dir;
        } else if (commands[0].arguments.size() == 1) {
            target_dir = commands[0].arguments[0];
        } else {
            std::cerr << "alpShell: cd: too many arguments" << std::endl;
            return true;
        }
        if (chdir(target_dir.c_str()) != 0) {
            perror(("alpShell: cd: " + target_dir).c_str());
        }
        return true; // Continue after cd attempt
    }


    // // --- Background Execution Check ---
    // bool runInBackground = false;
    // // Check the *original* line for '&' as the background check in main was simpler.
    // // A more robust solution involves the parser identifying '&' as a separate token/flag.
    // std::string trimmed_line = line;
    // size_t endpos = trimmed_line.find_last_not_of(" \t");
    // if (std::string::npos != endpos) {
    //     trimmed_line = trimmed_line.substr(0, endpos + 1);
    // }
    // if (!trimmed_line.empty() && trimmed_line.back() == '&') {
    //     runInBackground = true;
    //     // Only enable background execution if not explicitly disabled for scripts,
    //     // but standard shells allow background jobs in scripts.
    //     Command& last_cmd = commands.back();
    //     if (!last_cmd.arguments.empty() && last_cmd.arguments.back() == "&") {
    //          last_cmd.arguments.pop_back();
    //     } else if (last_cmd.arguments.empty() && last_cmd.executable == "&") {
    //          std::cerr << "alpShell: syntax error near unexpected token `&'" << std::endl;
    //          return true;
    //     }
    // }

    // --- Background Execution Check ---
    // Check the original line for '&'
    bool runInBackground = false;
    std::string original_trimmed = line; // Use original line for this check
    size_t endpos = original_trimmed.find_last_not_of(" \t");
    if (std::string::npos != endpos) {
        original_trimmed = original_trimmed.substr(0, endpos + 1);
    }
     if (!original_trimmed.empty() && original_trimmed.back() == '&') {
        runInBackground = true;
        Command& last_cmd = commands.back();
        if (!last_cmd.arguments.empty() && last_cmd.arguments.back() == "&") {
             last_cmd.arguments.pop_back();
        } else if (last_cmd.arguments.empty() && last_cmd.executable == "&") {
             std::cerr << "alpShell: syntax error near unexpected token `&'" << std::endl;
             return true;
        }
    }


    // --- Execute the pipeline ---
    pid_t last_pid = executePipeline(commands, runInBackground);


    // --- Add job to job control if running in background ---
    // Background jobs are usually allowed in scripts too
    if (runInBackground && last_pid > 0) {
        JobControl::addJob(last_pid, line);
        // Optionally suppress this message in non-interactive mode
        if (is_interactive) {
            std::cout << "Started background job [" << last_pid << "]" << std::endl;
        }
    } else if (runInBackground && last_pid == 0) {
         std::cerr << "Failed to start background job." << std::endl;
         // Decide script behavior: continue or exit?
         // return false; // Optionally exit script on background failure
    }

    // If a foreground command failed, should the script exit?
    // Standard shells often have `set -e` for this. We don't implement that yet.
    if (is_interactive)
    {
        rl_reset_line_state();
        rl_redisplay();
    }
    
    return true; // Continue processing
}


// void launch_debug_window()
// {
//     pid_t pid = fork();
//     if (pid == 0)
//     {
//         // Child process - launch new terminal window
//         execlp("xterm", "xterm", "-title", "alpShell Debug", "-e",
//                "tail -f debug.log", NULL);
//         perror("execlp");
//         exit(EXIT_FAILURE);
//     }
//     else if (pid < 0)
//     {
//         perror("fork");
//     }
// }

// --- Main Function ---
int main(int argc, char *argv[])
{
    // Add to beginning of main() before ANY other code
    // putenv((char *)"TERM=xterm-256color");
    setenv("TERM", "xterm-256color", 1);
    rl_event_hook = update_suggestion_hook;
    rl_variable_bind("visible-stats", "0");
    rl_variable_bind("colored-stats", "0");
    rl_variable_bind("enable-bracketed-paste", "1");
    rl_reset_terminal(nullptr); // Reinitialize terminal after env change

    rl_instream = stdin;
    rl_outstream = stdout;

    // Set completion and display hooks
    rl_attempted_completion_function = completion_callback;
    rl_redisplay_function = redisplay_hook;
    // rl_redisplay_hook = redisplay_hook;
    rl_char_is_quoted_p = reinterpret_cast<rl_linebuf_func_t *>(&is_quoted);

    // Then prepare terminal
    rl_prep_terminal(1); // Must come after stream assignments
    rl_set_signals();

    rl_bind_keyseq("\e[A", handleUpArrow);    // Up arrow
    rl_bind_keyseq("\e[B", handleDownArrow);  // Down arrow
    rl_bind_keyseq("\e[C", acceptSuggestion); // Right arrow

    // // Create debug log file
    // int log_fd = open("debug.log", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    // dup2(log_fd, STDERR_FILENO); // Redirect stderr to log

    // // Launch debug window if interactive
    // if (is_interactive)
    // {
    //     launch_debug_window();
    // }

    setenv("LS_COLORS",
           "di=36:"    // Directories cyan
           "*.h=33:"   // Header files yellow
           "*.cpp=32:" // C++ files green
           "*.sh=31:"  // Shell scripts red
           "*.txt=37:" // Text files white
           "ex=35:",
           1); // Executables magenta

    setenv("CLICOLOR", "1", 1);
    setenv("CLICOLOR_FORCE", "1", 1);

    // Setup signal handler for job control (needed in both modes)
    JobControl::setupSignalHandlers();

    bool is_interactive = false;
    std::istream* input_stream = &std::cin; // Default to stdin
    std::ifstream script_file;

    // Determine if running interactively or executing a script
    if (argc == 1) {
        // Check if stdin is connected to a terminal
        is_interactive = isatty(STDIN_FILENO);
    } else if (argc == 2) {
        // Attempt to open the script file
        script_file.open(argv[1]);
        if (!script_file.is_open()) {
            perror(("alpShell: Failed to open script file: " + std::string(argv[1])).c_str());
            return EXIT_FAILURE;
        }
        input_stream = &script_file; // Point input stream to the file
        is_interactive = false;
    } else {
        std::cerr << "Usage: " << argv[0] << " [script_file]" << std::endl;
        return EXIT_FAILURE;
    }

    // --- Main Processing Loop ---
    std::string line;
    char* read_buffer = nullptr; // Only for readline

    if (is_interactive) {
        // Interactive loop using readline
        while (true) {
            if (read_buffer) {
                free(read_buffer);
                read_buffer = nullptr;
            }
            read_buffer = readline(get_prompt().c_str()); // Dynamic prompt
            if (read_buffer == nullptr) { // EOF (Ctrl+D)
                std::cout << std::endl;
                break;
            }
            if (read_buffer && *read_buffer) {
                 add_history(read_buffer); // Add to readline history BEFORE processing
            }
            line = read_buffer;

            if (!process_line(line, is_interactive)) {
                 break; // process_line returned false (e.g., exit command)
            }
        }
        if (read_buffer) free(read_buffer); // Final cleanup
    } else {
        // Non-interactive loop reading from file or redirected stdin
        while (std::getline(*input_stream, line)) {
             if (!process_line(line, is_interactive)) {
                 break; // process_line returned false (e.g., exit command)
            }
        }
         // Check for errors reading from the file stream if needed
         // if (input_stream->bad()) { ... }
    }
    
    if (is_interactive)
    {
        // Force readline to recognize we're on a new line
        rl_on_new_line();
    }

    return EXIT_SUCCESS;
}