#ifndef COMMAND_H
#define COMMAND_H

#include <string>
#include <vector>

// Structure representing a shell command.
struct Command {
    std::string executable;           // The command, e.g., "ls"
    std::vector<std::string> arguments; // Command arguments
    std::string inputFile;            // Input redirection file (if any)
    std::string outputFile;           // Output redirection file (if any)
    bool appendOutput;                // Flag: true if output should be appended

    Command() : appendOutput(false) {}
};

// Prints the details of a Command to standard output.
void printCommand(const Command &cmd);

// Converts a Command to its string representation.
std::string commandToString(const Command &cmd);

#endif // COMMAND_H
