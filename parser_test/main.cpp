#include "parser.h"
#include "lexer.h"
#include <iostream>

int main() {
    std::string input = "ls -l | grep keyword > output.txt";
    Lexer lexer;
    std::vector<Token> tokens = lexer.tokenize(input);
    
    Parser parser;
    parser.parse(tokens);
    parser.printParseTree();
    
    return 0;
}

