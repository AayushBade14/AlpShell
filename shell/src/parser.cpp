// #include "parser.h"
// #include <iostream>

// std::vector<Command> Parser::parse(const std::vector<Token>& tokens) {
//     std::vector<Command> commands; 
//     root_.value = "Root";
    
//     Node* currentNode = &root_;
    
//     for (const auto& token : tokens) { 
//         Command cmd; // Create a Command object
//         if (token.type == TokenType::END) break;
        
//         Node* newNode = new Node(); // Allocate new Node dynamically
//         newNode->value = token.value; // Set value for the new Node
//         currentNode->children.push_back(newNode); // Add newNode to currentNode's children

//         if (token.type == TokenType::PIPE) { 
//             // Handle pipe command
//             cmd.executable = "|"; // Set executable for pipe command
//             Node* pipeNode = new Node(); // Allocate new Node dynamically for pipe
//             pipeNode->value = "|"; // Set value for the pipe Node
//             pipeNode->children.push_back(currentNode); // Add currentNode to pipeNode's children
//             currentNode = pipeNode; // Update currentNode to point to pipeNode
//             root_.children.push_back(pipeNode); // Add pipeNode to root's children

//         } else if (token.type == TokenType::REDIRECT_OUT || token.type == TokenType::REDIRECT_IN) {
//             // Handle redirection
//             Node* redirectNode = new Node(); // Allocate new Node dynamically for redirection
//             redirectNode->value = token.value; // Set value for the redirect Node
//             redirectNode->children.push_back(currentNode); // Add currentNode to redirectNode's children
//             currentNode = redirectNode; // Update currentNode to point to redirectNode
//             root_.children.push_back(redirectNode); // Add redirectNode to root's children

//         } else { 
//             // Add as child of current node
//             cmd.executable = token.value; // Set executable for the command
//             currentNode->children.push_back(newNode); // Add as child of current node
//             commands.push_back(cmd); // Add the command to the vector
//         }
//     }
//     return commands; // Return the vector of commands
// }

// void Parser::deleteNodes(Node* node) {
//     for (auto child : node->children) {
//         deleteNodes(child); // Recursively delete child nodes
//         delete child; // Delete the current node
//     }
// }

// Parser::~Parser() {
//     deleteNodes(&root_); // Clean up the root node and its children
// }

// void Parser::printParseTreeHelper(Node* node, int level) {
//     for (int i = 0; i < level; ++i) std::cout << "  ";
//     std::cout << node->value << std::endl;
    
//     for (const auto& child : node->children) {
//         printParseTreeHelper(child, level + 1);
//     }
// }

// void Parser::printParseTree() {
//     printParseTreeHelper(&root_, 0);
// }


#include "parser.h"
#include <iostream>
#include <stdexcept> // For potential error reporting (though not strictly used here)
#include <vector>
#include <string>

// --- Helper Function Prototypes within this file ---
// (These operate on the Node structure defined in parser.h)
static void flattenNodeTree(const Parser::Node* node, std::vector<Command>& commands);


// --- Parser Member Function Definitions ---

// Public parse method: Builds the internal Node tree and then flattens it.
std::vector<Command> Parser::parse(const std::vector<Token>& tokens) {
    // 1. Clear any previous tree structure hanging off root_
    deleteNodes(&root_); // Uses the class's deleteNodes method
    root_.value = "Root"; // Reset root node value
    // root_.children is implicitly cleared by deleteNodes

    if (tokens.empty() || tokens[0].type == TokenType::END) {
        return {}; // Return empty vector for empty input
    }

    // 2. Build the internal Node tree representation under root_
    Node* currentCommandNode = nullptr;

    for (size_t i = 0; i < tokens.size(); ++i) {
        const Token& token = tokens[i];

        if (token.type == TokenType::END) {
            break;
        }

        if (token.type == TokenType::PIPE) {
            if (currentCommandNode == nullptr) {
                std::cerr << "Parser Error: Pipe '|' without preceding command." << std::endl;
                deleteNodes(&root_); // Clean up potentially partial tree
                return {}; // Return empty on error
            }
            currentCommandNode = nullptr; // Signal that the next IDENTIFIER starts a new command
            continue; // Move to next token
        }

        // If currentCommandNode is null, the next IDENTIFIER starts a new command segment
        if (currentCommandNode == nullptr) {
            if (token.type != TokenType::IDENTIFIER) {
                 std::cerr << "Parser Error: Expected command name, got token '" << token.value << "'" << std::endl;
                 deleteNodes(&root_);
                 return {};
            }
            // Create a new command node and add it as a child of the root
            currentCommandNode = new Node();
            currentCommandNode->value = token.value; // Executable name
            root_.children.push_back(currentCommandNode);
        } else {
            // We are inside a command segment (currentCommandNode is not null)
            if (token.type == TokenType::IDENTIFIER) {
                // Treat as argument or filename after redirection
                Node* argNode = new Node();
                argNode->value = token.value;
                currentCommandNode->children.push_back(argNode);
            } else if (token.type == TokenType::REDIRECT_IN || token.type == TokenType::REDIRECT_OUT) {
                 // Add redirection operator node
                 Node* redirectNode = new Node();
                 redirectNode->value = token.value; // Store ">" or "<"
                 currentCommandNode->children.push_back(redirectNode);

                 // The very next token MUST be the filename identifier
                 if (i + 1 >= tokens.size() || tokens[i + 1].type != TokenType::IDENTIFIER) {
                    std::cerr << "Parser Error: Expected filename after redirection operator '" << token.value << "'." << std::endl;
                    deleteNodes(&root_);
                    return {};
                 }
                 // Consume the filename token now and add it as a node
                 i++; // Advance the outer loop index
                 Node* filenameNode = new Node();
                 filenameNode->value = tokens[i].value;
                 currentCommandNode->children.push_back(filenameNode);

            } else {
                 // Should not happen with the current lexer tokens (PIPE/END handled earlier)
                 std::cerr << "Parser Warning: Unexpected token type encountered: " << token.value << std::endl;
            }
        }
    }

    // Optional: Print the internal tree for debugging
    // printParseTree();

    // 3. Flatten the built Node tree into the vector<Command> format
    std::vector<Command> commandList;
    flattenNodeTree(&root_, commandList); // Use the static helper

    return commandList;
}

// Destructor: Cleans up the node tree
Parser::~Parser() {
    deleteNodes(&root_);
}

// Helper to recursively delete nodes (implementation)
// Deletes children first, then the node itself IF it was dynamically allocated.
// Important: It only deletes the children of the node passed to it.
void Parser::deleteNodes(Node* node) {
    if (!node) return;

    // Iterate through children, recursively delete them, and free memory
    for (Node* child : node->children) {
        deleteNodes(child); // Delete grandchildren etc. first
        delete child;       // Delete the child node itself (allocated with new)
    }
    // Clear the vector of pointers after deleting the pointed-to objects
    node->children.clear();

    // DO NOT delete 'node' itself here, as root_ is on the stack
    // and children are deleted by their parent in the loop above.
}


// Public method to print the parse tree
void Parser::printParseTree() {
    std::cout << "--- Parse Tree ---" << std::endl;
    printParseTreeHelper(&root_, 0);
    std::cout << "------------------" << std::endl;
}

// Recursive helper to print the tree with indentation (implementation)
void Parser::printParseTreeHelper(Node* node, int level) {
     if (!node) return; // Should not happen with root_ but good practice

    // Indentation
    for (int i = 0; i < level; ++i) {
        std::cout << "  ";
    }

    // Print node value
    std::cout << node->value << std::endl;

    // Recursively print children
    for (const auto& child : node->children) {
        printParseTreeHelper(child, level + 1);
    }
}


// --- Static Helper Function Definition ---

// Flattens the Node tree (hanging off root_) into the vector<Command>
// This function understands the structure built by Parser::parse
static void flattenNodeTree(const Parser::Node* node, std::vector<Command>& commands) {
    if (!node || node->value != "Root") { // Expecting the root node
        return;
    }

    // Iterate through the direct children of Root - each should represent a command segment
    for (const Parser::Node* commandNode : node->children) {
        Command currentCmd;
        if (!commandNode) continue; // Safety check

        // The commandNode's value is the executable
        currentCmd.executable = commandNode->value;

        // Iterate through the children of the commandNode for arguments and redirections
        bool expectOutputFilename = false;
        bool expectInputFilename = false;
        bool appendOutput = false; // Not handled by this simple lexer/parser structure

        for (const Parser::Node* childNode : commandNode->children) {
             if (!childNode) continue; // Safety check

            if (expectOutputFilename) {
                currentCmd.outputFile = childNode->value;
                currentCmd.appendOutput = appendOutput; // Set based on '>' or potentially '>>'
                expectOutputFilename = false;
                appendOutput = false; // Reset
            } else if (expectInputFilename) {
                currentCmd.inputFile = childNode->value;
                expectInputFilename = false;
            } else {
                // If not expecting a filename, check for redirection operator or argument
                if (childNode->value == ">") {
                     if (!currentCmd.outputFile.empty()) {
                         std::cerr << "Flatten Error: Multiple output redirections for command: " << currentCmd.executable << std::endl;
                         // Optionally clear 'commands' and return to signal error
                     }
                    expectOutputFilename = true;
                    appendOutput = false; // Default for '>'
                } else if (childNode->value == "<") {
                     if (!currentCmd.inputFile.empty()) {
                        std::cerr << "Flatten Error: Multiple input redirections for command: " << currentCmd.executable << std::endl;
                        // Optionally clear 'commands' and return to signal error
                     }
                    expectInputFilename = true;
                } else {
                    // Otherwise, it's an argument
                    currentCmd.arguments.push_back(childNode->value);
                }
            }
        }
         // Check for dangling redirections after processing children
        if (expectInputFilename || expectOutputFilename) {
             std::cerr << "Flatten Error: Command '" << currentCmd.executable << "' ends with redirection operator without filename." << std::endl;
             // Optionally clear 'commands' and return to signal error
        }


        // Add the constructed command to the list
        commands.push_back(currentCmd);
    }
}