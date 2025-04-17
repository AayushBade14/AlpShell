// #include "executor.h"
// #include <unistd.h>
// #include <sys/wait.h>
// #include <iostream>
// #include <vector>

// void executeCommand(const Command &cmd) {
//     pid_t pid = fork();
//     if (pid == 0) { // Child process
//         // Handle input redirection if specified
//         if (!cmd.inputFile.empty()) {
//             FILE *in = freopen(cmd.inputFile.c_str(), "r", stdin);
//             if (!in) {
//                 std::cerr << "Error: Unable to open input file: " << cmd.inputFile << std::endl;
//                 exit(EXIT_FAILURE);
//             }
//         }
//         // Handle output redirection if specified
//         if (!cmd.outputFile.empty()) {
//             const char* mode = cmd.appendOutput ? "a" : "w";
//             FILE *out = freopen(cmd.outputFile.c_str(), mode, stdout);
//             if (!out) {
//                 std::cerr << "Error: Unable to open output file: " << cmd.outputFile << std::endl;
//                 exit(EXIT_FAILURE);
//             }
//         }
        
//         // Build argument list for execvp
//         std::vector<char*> args;
//         args.push_back(const_cast<char*>(cmd.executable.c_str()));
//         for (auto &arg : cmd.arguments) {
//             args.push_back(const_cast<char*>(arg.c_str()));
//         }
//         args.push_back(nullptr); // execvp expects a null-terminated array

//         if (execvp(cmd.executable.c_str(), args.data()) == -1) {
//             std::cerr << "Error: Execution failed for command: " << cmd.executable << std::endl;
//             exit(EXIT_FAILURE);
//         }
//     } else if (pid < 0) {
//         std::cerr << "Error: Fork failed!" << std::endl;
//     } else { // Parent process
//         int status;
//         waitpid(pid, &status, 0);
//     }
// }


#include "executor.h"
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h> // For open() flags if switching from freopen
#include <iostream>
#include <vector>
#include <cstdio>   // For perror
#include <cstdlib>  // For exit, EXIT_SUCCESS, EXIT_FAILURE

// Helper function to execute a single command within a child process
// This is essentially the core logic moved from the old executeCommand
// Handles I/O setup and execvp. Called by executePipeline after forking.
static void runChildCommand(const Command& cmd) {
    // Handle input redirection if specified
    if (!cmd.inputFile.empty()) {
        // Using open/dup2 is generally safer than freopen in complex scenarios
        int fd_in = open(cmd.inputFile.c_str(), O_RDONLY);
        if (fd_in < 0) {
            perror(("Error opening input file: " + cmd.inputFile).c_str());
            exit(EXIT_FAILURE);
        }
        if (dup2(fd_in, STDIN_FILENO) < 0) {
             perror("Error redirecting stdin");
             close(fd_in); // Close file descriptor on error
             exit(EXIT_FAILURE);
        }
        close(fd_in); // Close original fd after dup2
    }

    // Handle output redirection if specified
    if (!cmd.outputFile.empty()) {
        int flags = O_WRONLY | O_CREAT;
        if (cmd.appendOutput) {
            flags |= O_APPEND;
        } else {
            flags |= O_TRUNC; // Overwrite
        }
        // File permissions (e.g., rw-r--r--)
        mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
        int fd_out = open(cmd.outputFile.c_str(), flags, mode);
        if (fd_out < 0) {
            perror(("Error opening output file: " + cmd.outputFile).c_str());
            exit(EXIT_FAILURE);
        }
         if (dup2(fd_out, STDOUT_FILENO) < 0) {
             perror("Error redirecting stdout");
             close(fd_out);
             exit(EXIT_FAILURE);
         }
        close(fd_out);
    }

    // Build argument list for execvp
    std::vector<char*> args;
    args.push_back(const_cast<char*>(cmd.executable.c_str()));
    for (const auto& arg : cmd.arguments) { // Use const auto&
        args.push_back(const_cast<char*>(arg.c_str()));
    }
    args.push_back(nullptr); // execvp expects a null-terminated array

    // Execute the command
    if (execvp(cmd.executable.c_str(), args.data()) == -1) {
        perror(("Error executing command: " + cmd.executable).c_str());
        exit(EXIT_FAILURE); // Exit child if exec fails
    }
    // Should not reach here if execvp succeeds
}


// --- New Pipeline Execution Function ---
pid_t executePipeline(const std::vector<Command>& commands, bool background) {
    int numCommands = commands.size();
    if (numCommands == 0) {
        return 0; // Nothing to execute
    }

    int prev_pipe_read_end = -1; // Read end of the pipe from the *previous* command
    std::vector<pid_t> child_pids;
    pid_t last_pid = 0;

    for (int i = 0; i < numCommands; ++i) {
        int pipefds[2]; // pipefds[0] = read end, pipefds[1] = write end

        // Create a pipe for all commands except the last one
        if (i < numCommands - 1) {
            if (pipe(pipefds) < 0) {
                perror("Pipe creation failed");
                // Cleanup: kill already created children? Complex, skip for now.
                return 0; // Indicate failure
            }
        }

        pid_t pid = fork();

        if (pid < 0) {
            perror("Fork failed");
            // Cleanup? Close pipe ends?
            if (i < numCommands - 1) {
                close(pipefds[0]);
                close(pipefds[1]);
            }
            if (prev_pipe_read_end != -1) {
                 close(prev_pipe_read_end);
            }
            return 0; // Indicate failure
        }

        if (pid == 0) { // --- Child Process ---
            // Redirect standard input if not the first command
            if (prev_pipe_read_end != -1) {
                if (dup2(prev_pipe_read_end, STDIN_FILENO) < 0) {
                    perror("Child: Failed to redirect stdin from pipe");
                    exit(EXIT_FAILURE);
                }
                close(prev_pipe_read_end); // Close original read end
            }

            // Redirect standard output if not the last command
            if (i < numCommands - 1) {
                 // Close the read end of the *current* pipe, child doesn't need it
                 close(pipefds[0]);
                if (dup2(pipefds[1], STDOUT_FILENO) < 0) {
                    perror("Child: Failed to redirect stdout to pipe");
                     close(pipefds[1]); // Close write end on error
                    exit(EXIT_FAILURE);
                }
                close(pipefds[1]); // Close original write end after dup2
            }

            // Now handle file redirection specific to this command
            // (This will override pipe redirection if specified for first/last command)
            // Use the helper function which includes file redirection logic
            runChildCommand(commands[i]);

            // runChildCommand exits on failure, so we should only reach here if execvp fails inside it.
            // It already prints an error and exits.
            exit(EXIT_FAILURE); // Just in case runChildCommand logic changes

        } else { // --- Parent Process ---
            last_pid = pid; // Keep track of the last PID for background jobs/return value
            child_pids.push_back(pid);

             // Close the read end of the previous pipe (parent doesn't need it)
            if (prev_pipe_read_end != -1) {
                close(prev_pipe_read_end);
            }

            // Close the write end of the current pipe (parent doesn't need it)
            if (i < numCommands - 1) {
                close(pipefds[1]);
                prev_pipe_read_end = pipefds[0]; // Save the read end for the *next* child
            }
        }
    } // --- End of loop through commands ---

    // If running in the foreground, wait for all children
    if (!background) {
        int status;
        for (pid_t child_pid : child_pids) {
            // WUNTRACED allows catching stopped jobs (Ctrl+Z), though not fully handled yet
            waitpid(child_pid, &status, WUNTRACED);
            // Could check WIFEXITED, WEXITSTATUS, WIFSIGNALED, etc. here if needed
        }
    }
    // If running in background, the parent returns immediately (caller adds job)

    return last_pid; // Return PID of the last command
}

// --- Old executeCommand can be removed or kept if needed elsewhere ---
// For simplicity, let's assume executePipeline handles the single command case.
// If you *need* executeCommand specifically, you can make it call executePipeline:
void executeCommand(const Command& cmd) {
     std::vector<Command> single_command_pipeline;
     single_command_pipeline.push_back(cmd);
     executePipeline(single_command_pipeline, false); // Execute single command in foreground
}