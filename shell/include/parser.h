// #ifndef PARSER_H
// #define PARSER_H

// #include <vector>
// #include "command.h"
// #include "lexer.h"

// class Parser {
// public:
//     ~Parser(); // Destructor declaration
//     std::vector<Command> parse(const std::vector<Token>& tokens);
//     void printParseTree();
//     struct Node {
//         std::string value;
//         std::vector<Node*> children; // Store pointers to Node
//     };
//     void printParseTreeHelper(Node* node, int level);
// private:
//     Node root_;
//     void deleteNodes(Node* node); // Added helper function declaration
// };

// #endif


#ifndef PARSER_H
#define PARSER_H

#include <vector>
#include <string>
#include "command.h"
#include "lexer.h" // Include lexer for QuotingType

class Parser {
public:
    // Make Node public to allow flattenNodeTree access
    struct Node {
        std::string value;
        QuotingType quoting = QuotingType::NONE; // Store quoting info
        std::vector<Node*> children;
    };

// Public methods
    ~Parser();
    std::vector<Command> parse(const std::vector<Token>& tokens);
    void printParseTree();
    void printParseTreeHelper(Node* node, int level);

private:
    // Private members
    Node root_;
    void deleteNodes(Node* node);
};

#endif // PARSER_H