#include "executor.h"
#include <unistd.h>
#include <sys/wait.h>
#include <iostream>
#include <vector>

void executeCommand(const Command &cmd) {
    std::cout << "Executing command: " << cmd.executable << std::endl; // Debug statement
    std::cout << "Arguments: ";
    for (const auto& arg : cmd.arguments) {
        std::cout << arg << " ";
    }
    std::cout << std::endl; // Log the arguments
    if (cmd.executable.empty()) { 
        std::cerr << "Error: No command provided." << std::endl; // Debug statement
        std::cerr << "Error: No command provided." << std::endl;
        return;
    }
    pid_t pid = fork();
    if (pid < 0) {
        std::cerr << "Error: Fork failed!" << std::endl;
        return;
    }
    if (pid == 0) { // Child process
        // Handle input redirection if specified
        if (!cmd.inputFile.empty()) {
            FILE *in = freopen(cmd.inputFile.c_str(), "r", stdin);
            if (!in) {
                std::cerr << "Error: Unable to open input file: " << cmd.inputFile << std::endl;
                exit(EXIT_FAILURE);
            }
        }
        // Handle output redirection if specified
        if (!cmd.outputFile.empty()) {
            const char* mode = cmd.appendOutput ? "a" : "w";
            FILE *out = freopen(cmd.outputFile.c_str(), mode, stdout);
            if (!out) {
                std::cerr << "Error: Unable to open output file: " << cmd.outputFile << std::endl;
                exit(EXIT_FAILURE);
            }
        }
        
        // Build argument list for execvp
        std::vector<char*> args;
        args.push_back(const_cast<char*>(cmd.executable.c_str()));
        for (auto &arg : cmd.arguments) {
            args.push_back(const_cast<char*>(arg.c_str()));
        }
        args.push_back(nullptr); // execvp expects a null-terminated array

        if (execvp(cmd.executable.c_str(), args.data()) == -1) {
            std::cerr << "Error: Execution failed for command: " << cmd.executable << std::endl;
            exit(EXIT_FAILURE);
        }
    } else { // Parent process
        int status;
        waitpid(pid, &status, 0);
    }
}
