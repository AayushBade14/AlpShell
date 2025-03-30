#include "command.h"
#include <iostream>
#include <sstream>

// Function to print the details of a Command
void printCommand(const Command &cmd) {
    std::cout << "Executable: " << cmd.executable << std::endl;
    
    std::cout << "Arguments: ";
    for (const auto &arg : cmd.arguments) {
        std::cout << arg << " ";
    }
    std::cout << std::endl;
    
    if (!cmd.inputFile.empty()) {
        std::cout << "Input Redirection: " << cmd.inputFile << std::endl;
    }
    if (!cmd.outputFile.empty()) {
        std::cout << "Output Redirection: " 
                  << cmd.outputFile 
                  << (cmd.appendOutput ? " (append)" : " (overwrite)") 
                  << std::endl;
    }
}

// Function to convert a Command to a string representation
std::string commandToString(const Command &cmd) {
    std::ostringstream oss;
    oss << cmd.executable;
    
    for (const auto &arg : cmd.arguments) {
        oss << " " << arg;
    }
    
    if (!cmd.inputFile.empty()) {
        oss << " < " << cmd.inputFile;
    }
    
    if (!cmd.outputFile.empty()) {
        oss << (cmd.appendOutput ? " >> " : " > ") << cmd.outputFile;
    }
    
    return oss.str();
}

// Additional functions can be added here, for example:
// - A function to validate a command structure.
// - A function to split a command string into parts (if not handled by the parser).
// - Helper functions to modify or combine commands.
