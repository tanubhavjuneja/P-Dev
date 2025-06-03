#pragma once
#include "lexer.h"
#include <map>
#include <string>
#include <vector>
#include <variant>
#include <stack>
class Parser {
public:
    Parser(Lexer lexer);
    void parse();
    int parseExpression();
    void statement();
private:
    struct FunctionInfo {
        size_t position;
        std::string param;
    };
    using Value = std::variant<int, std::string>;
    std::map<std::string, Value> variables;
    std::map<std::string, FunctionInfo> functions;
    std::stack<std::pair<size_t, Token>> returnStates;
    Lexer lexer;
    Token currentToken;
    void consume(Token::Type expected);
    int expr();
    int term();
    int factor();
};