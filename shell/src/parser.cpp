#include "parser.h"
#include <iostream>
#include <stdexcept>
#include <vector>
#include <string>
#include <cstdlib> // For getenv()
#include <cctype>  // For isalpha, isalnum, etc.
#include <glob.h>

// --- Forward Declarations for Static Helpers ---
static void flattenNodeTree(const Parser::Node* node, std::vector<Command>& commands);
static std::string expandVariables(const std::string& input, QuotingType quoting);
static bool containsWildcard(const std::string& s);


// --- Variable Expansion Helper ---
static std::string expandVariables(const std::string& input, QuotingType quoting) {
    // No expansion inside single quotes
    if (quoting == QuotingType::SINGLE) {
        return input;
    }

    std::string result;
    result.reserve(input.length()); // Optimization

    for (size_t i = 0; i < input.length(); ++i) {
        if (input[i] == '$') {
            // Check for variable start
            if (i + 1 < input.length()) {
                size_t start_var_name = i + 1;
                size_t end_var_name = start_var_name;
                std::string var_name;

                if (input[start_var_name] == '{') {
                    // Case: ${VAR}
                    start_var_name++; // Skip '{'
                    end_var_name = input.find('}', start_var_name);
                    if (end_var_name == std::string::npos) {
                        // Error: Unmatched '{', treat '$' literally
                        result += '$';
                        continue; // Skip rest of variable processing
                    }
                    var_name = input.substr(start_var_name, end_var_name - start_var_name);
                    i = end_var_name; // Move main index past '}'
                } else if (isalpha(input[start_var_name]) || input[start_var_name] == '_') {
                    // Case: $VAR
                    end_var_name = start_var_name;
                    while (end_var_name < input.length() &&
                           (isalnum(input[end_var_name]) || input[end_var_name] == '_')) {
                        end_var_name++;
                    }
                    var_name = input.substr(start_var_name, end_var_name - start_var_name);
                    i = end_var_name - 1; // Move main index past last char of var name
                } else {
                    // Not a valid variable start (e.g., $$, $?, $!) - treat $ literally for now
                    result += '$';
                    continue;
                }

                // Lookup variable and append value (or empty string if not found)
                const char* var_value = getenv(var_name.c_str());
                if (var_value != nullptr) {
                    result += var_value;
                }
                // If var_value is null, we append nothing (empty string)

            } else {
                // '$' is the last character, treat literally
                result += '$';
            }
        } else {
            // Not a '$', just append the character
            result += input[i];
        }
    } // End character loop

    return result;
}

static bool containsWildcard(const std::string& s) {
    // Basic check: More sophisticated checks could avoid matching literal brackets etc.
    return s.find('*') != std::string::npos ||
           s.find('?') != std::string::npos ||
           s.find('[') != std::string::npos;
}


// --- Parser Member Function Definitions ---

std::vector<Command> Parser::parse(const std::vector<Token>& tokens) {
    deleteNodes(&root_);
    root_.value = "Root";
    root_.quoting = QuotingType::NONE; // Root node doesn't represent a quoted token

    if (tokens.empty() || tokens[0].type == TokenType::END) {
        return {};
    }

    Node* currentCommandNode = nullptr;

    for (size_t i = 0; i < tokens.size(); ++i) {
        const Token& token = tokens[i];

        if (token.type == TokenType::END) {
            break;
        }

        if (token.type == TokenType::PIPE) {
            if (currentCommandNode == nullptr) {
                std::cerr << "Parser Error: Pipe '|' without preceding command." << std::endl;
                deleteNodes(&root_);
                return {};
            }
            currentCommandNode = nullptr;
            continue;
        }

        if (currentCommandNode == nullptr) { // Start of a new command segment
            if (token.type != TokenType::IDENTIFIER) {
                 std::cerr << "Parser Error: Expected command name, got token '" << token.value << "'" << std::endl;
                 deleteNodes(&root_);
                 return {};
            }
            currentCommandNode = new Node();
            currentCommandNode->value = token.value;
            currentCommandNode->quoting = token.quoting; // *** Store quoting info ***
            root_.children.push_back(currentCommandNode);
        } else { // Inside a command segment (argument or redirection)
            if (token.type == TokenType::IDENTIFIER) {
                // Argument or filename
                Node* argNode = new Node();
                argNode->value = token.value;
                argNode->quoting = token.quoting; // *** Store quoting info ***
                currentCommandNode->children.push_back(argNode);
            } else if (token.type == TokenType::REDIRECT_IN || token.type == TokenType::REDIRECT_OUT ||
              token.type == TokenType::REDIRECT_APPEND) {

                 Node* redirectNode = new Node();
                 redirectNode->value = token.value; // Store ">" or "<"
                 redirectNode->quoting = QuotingType::NONE; // Operators aren't quoted
                 currentCommandNode->children.push_back(redirectNode);

                 if (i + 1 >= tokens.size() || tokens[i + 1].type != TokenType::IDENTIFIER) {
                    std::cerr << "Parser Error: Expected filename after redirection operator '" << token.value << "'." << std::endl;
                    deleteNodes(&root_);
                    return {};
                 }
                 i++; // Consume filename token
                 Node* filenameNode = new Node();
                 filenameNode->value = tokens[i].value;
                 filenameNode->quoting = tokens[i].quoting; // *** Store quoting info ***
                 currentCommandNode->children.push_back(filenameNode);
            } else {
                 std::cerr << "Parser Warning: Unexpected token type encountered: " << token.value << std::endl;
            }
        }
    }

    // Flatten the tree (flattenNodeTree now does expansion)
    std::vector<Command> commandList;
    flattenNodeTree(&root_, commandList);

    return commandList;
}

Parser::~Parser() {
    deleteNodes(&root_);
}

void Parser::deleteNodes(Node* node) {
    if (!node) return;
    for (Node* child : node->children) {
        deleteNodes(child);
        delete child;
    }
    node->children.clear();
}

void Parser::printParseTree() {
    std::cout << "--- Parse Tree (Before Expansion) ---" << std::endl;
    printParseTreeHelper(&root_, 0);
    std::cout << "-------------------------------------" << std::endl;
}

void Parser::printParseTreeHelper(Node* node, int level) {
     if (!node) return;
    for (int i = 0; i < level; ++i) { std::cout << "  "; }
    // Optionally show quoting type for debugging
    const char* q_type = (node->quoting == QuotingType::SINGLE) ? "[S]" :
                         (node->quoting == QuotingType::DOUBLE) ? "[D]" : "[N]";
    std::cout << node->value << " " << q_type << std::endl;
    for (const auto& child : node->children) {
        printParseTreeHelper(child, level + 1);
    }
}


// --- Static Helper Function Definition ---

// Flattens the Node tree AND performs variable expansion
// static void flattenNodeTree(const Parser::Node* node, std::vector<Command>& commands) {
//     if (!node || node->value != "Root") { return; }

//     for (const Parser::Node* commandNode : node->children) {
//         Command currentCmd;
//         if (!commandNode) continue;

//         // *** Expand executable name ***
//         currentCmd.executable = expandVariables(commandNode->value, commandNode->quoting);

//         bool expectOutputFilename = false;
//         bool expectInputFilename = false;
//         bool appendOutput = false; // Still not handled

//         for (const Parser::Node* childNode : commandNode->children) {
//              if (!childNode) continue;

//             if (expectOutputFilename) {
//                  // *** Expand output filename ***
//                 currentCmd.outputFile = expandVariables(childNode->value, childNode->quoting);
//                 currentCmd.appendOutput = appendOutput;
//                 expectOutputFilename = false;
//                 appendOutput = false;
//             } else if (expectInputFilename) {
//                  // *** Expand input filename ***
//                 currentCmd.inputFile = expandVariables(childNode->value, childNode->quoting);
//                 expectInputFilename = false;
//             } else {
//                 if (childNode->value == ">") { // Operators themselves are not expanded
//                     if (!currentCmd.outputFile.empty()) { /* Error */ }
//                     expectOutputFilename = true;
//                     appendOutput = false;
//                 } else if (childNode->value == "<") { // Operators themselves are not expanded
//                     if (!currentCmd.inputFile.empty()) { /* Error */ }
//                     expectInputFilename = true;
//                 } else {
//                     // Otherwise, it's an argument - expand it
//                     currentCmd.arguments.push_back(expandVariables(childNode->value, childNode->quoting));
//                 }
//             }
//         }
//         if (expectInputFilename || expectOutputFilename) { /* Dangling redirection Error */ }

//         // Add the constructed & expanded command
//         commands.push_back(currentCmd);
//     }
// }

static void flattenNodeTree(const Parser::Node* node, std::vector<Command>& commands) {
    // if (!node || node->value != "Root") { return; }

    // for (const Parser::Node* commandNode : node->children) {
    //     Command currentCmd;
    //     if (!commandNode) continue;

    //     currentCmd.executable = expandVariables(commandNode->value, commandNode->quoting);

    //     bool expectOutputFilename = false;
    //     bool expectInputFilename = false;
    //     bool appendOutput = false; // Reset append flag for each command segment

    //     for (const Parser::Node* childNode : commandNode->children) {
    //          if (!childNode) continue;

    //         if (expectOutputFilename) {
    //             currentCmd.outputFile = expandVariables(childNode->value, childNode->quoting);
    //             // *** Use the appendOutput flag set when the operator was seen ***
    //             currentCmd.appendOutput = appendOutput;
    //             expectOutputFilename = false;
    //             // appendOutput is automatically reset later if another > or >> is seen
    //         } else if (expectInputFilename) {
    //             currentCmd.inputFile = expandVariables(childNode->value, childNode->quoting);
    //             expectInputFilename = false;
    //         } else {
    //             // Check for redirection operators FIRST
    //             if (childNode->value == ">") {
    //                 if (!currentCmd.outputFile.empty()) { /* Error: Multiple output redirects */ }
    //                 expectOutputFilename = true;
    //                 appendOutput = false; // Set append flag to false for >
    //             } else if (childNode->value == ">>") { // *** Check for >> ***
    //                  if (!currentCmd.outputFile.empty()) { /* Error: Multiple output redirects */ }
    //                 expectOutputFilename = true;
    //                 appendOutput = true; // Set append flag to true for >>
    //             } else if (childNode->value == "<") {
    //                 if (!currentCmd.inputFile.empty()) { /* Error: Multiple input redirects */ }
    //                 expectInputFilename = true;
    //             } else {
    //                 // Otherwise, it's an argument - expand it
    //                 currentCmd.arguments.push_back(expandVariables(childNode->value, childNode->quoting));
    //             }
    //         }
    //     }
    //     if (expectInputFilename || expectOutputFilename) { /* Dangling redirection Error */ }

    //     commands.push_back(currentCmd);
    // }

    if (!node || node->value != "Root") { return; }

    for (const Parser::Node* commandNode : node->children) {
        Command currentCmd;
        if (!commandNode) continue;

        // Expand executable FIRST (usually no globbing on executable itself)
        currentCmd.executable = expandVariables(commandNode->value, commandNode->quoting);

        bool expectOutputFilename = false;
        bool expectInputFilename = false;
        bool appendOutput = false;

        for (const Parser::Node* childNode : commandNode->children) {
             if (!childNode) continue;

            // --- Handle Filenames for Redirection ---
            if (expectOutputFilename) {
                std::string filename = expandVariables(childNode->value, childNode->quoting);
                // NOTE: Generally, we DON'T glob redirection filenames in shells like bash.
                // If you wanted globbing here, you'd add the glob logic, but it's non-standard.
                currentCmd.outputFile = filename;
                currentCmd.appendOutput = appendOutput;
                expectOutputFilename = false;
                appendOutput = false;
                continue; // Move to next child node
            } else if (expectInputFilename) {
                std::string filename = expandVariables(childNode->value, childNode->quoting);
                // Don't glob input filenames either
                currentCmd.inputFile = filename;
                expectInputFilename = false;
                continue; // Move to next child node
            }

            // --- Handle Redirection Operators ---
            if (childNode->value == ">" || childNode->value == ">>" || childNode->value == "<") {
                 if (childNode->value == ">") {
                     if (!currentCmd.outputFile.empty()) { /* Error */ }
                     expectOutputFilename = true;
                     appendOutput = false;
                 } else if (childNode->value == ">>") {
                     if (!currentCmd.outputFile.empty()) { /* Error */ }
                     expectOutputFilename = true;
                     appendOutput = true;
                 } else if (childNode->value == "<") {
                     if (!currentCmd.inputFile.empty()) { /* Error */ }
                     expectInputFilename = true;
                 }
                 continue; // Operator handled, move to next child node
            }

            // --- Handle Regular Arguments (Potential Globbing) ---

            // 1. Expand variables first
            std::string potentialArg = expandVariables(childNode->value, childNode->quoting);

            // 2. Check if globbing should be applied
            //    - Only if NOT single/double quoted AND contains wildcard characters
            if (childNode->quoting == QuotingType::NONE && containsWildcard(potentialArg))
            {
                glob_t glob_result;
                // Use GLOB_NOCHECK: If no match, returns the pattern itself.
                // Use GLOB_TILDE: Handle ~ expansion (optional but useful)
                // GLOB_ERR: Report errors
                int glob_status = glob(potentialArg.c_str(), GLOB_TILDE | GLOB_NOCHECK | GLOB_ERR, nullptr, &glob_result);

                if (glob_status == 0) {
                     // Glob succeeded, add all matches (could be 1 if GLOB_NOCHECK returned pattern)
                     for (size_t i = 0; i < glob_result.gl_pathc; ++i) {
                         currentCmd.arguments.push_back(glob_result.gl_pathv[i]);
                     }
                } else {
                    // Glob failed (error or GLOB_NOMATCH without GLOB_NOCHECK)
                    // With GLOB_NOCHECK, glob_status != 0 usually means an error like GLOB_NOSPACE or GLOB_ABORTED.
                    // We'll just add the unglobbed argument as a fallback.
                    // You could print an error here based on glob_status if desired.
                     if (glob_status == GLOB_NOMATCH) {
                         // Should not happen with GLOB_NOCHECK, but defensively add original arg
                         currentCmd.arguments.push_back(potentialArg);
                     } else {
                         std::cerr << "alpShell: glob error: " << glob_status << " for pattern " << potentialArg << std::endl;
                         // Still add the original argument so the command receives something
                         currentCmd.arguments.push_back(potentialArg);
                     }
                }
                // Free memory allocated by glob()
                globfree(&glob_result);
            }
            else {
                // No globbing needed (quoted or no wildcards)
                currentCmd.arguments.push_back(potentialArg);
            }

        } // End loop through children (arguments/redirections)

        if (expectInputFilename || expectOutputFilename) { /* Dangling redirection Error */ }

        commands.push_back(currentCmd);
    }
}